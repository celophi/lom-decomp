/** @file field_mesh_part_animation.c
 * @brief Copy matrix rotations and animate the VRAM texture parts of actor meshes.
 */

#include "common.h"
#include "field_effect_types.h"
#include "field_actor_runtime.h"
#include "field_mesh.h"
#include "field_mesh_transform.h"
#include "sdk/libgpu.h"

/** @brief First part effect kind that renders a mesh; kinds up to FIELD_PART_MESH_LAST select meshes 2..0. */
#define FIELD_PART_MESH_FIRST 0xF7
/** @brief Part effect kind that renders mesh 0. */
#define FIELD_PART_MESH_LAST 0xF9
/** @brief Number of mesh effect kinds. */
#define FIELD_PART_MESH_KIND_COUNT 3

/** @brief Texture scroll directions accepted by func_8008343C. */
enum
{
    FIELD_MESH_TEXTURE_SCROLL_UP = 0,
    FIELD_MESH_TEXTURE_SCROLL_DOWN = 1,
    FIELD_MESH_TEXTURE_SCROLL_LEFT = 2,
    FIELD_MESH_TEXTURE_SCROLL_RIGHT = 3
};

/** @brief VRAM column of the first actor-owned mesh texture page. */
#define FIELD_MESH_TEXTURE_OWNER_VRAM_X 0x340
/** @brief VRAM row of the actor-owned mesh texture pages. */
#define FIELD_MESH_TEXTURE_OWNER_VRAM_Y 0x100
/** @brief VRAM column of the shared mesh texture page. */
#define FIELD_MESH_TEXTURE_SHARED_VRAM_X 0x140

/** @brief Rotation part of a MATRIX (the 3x3 terms and padding) as five words. */
typedef struct
{
    s32 words[5];
} FieldMatrixRotationWords;

s32 func_8008343C(s32 mesh_index, s32 direction, FieldActorState *actor, s32 shift);
void func_80083868(s32 mesh_index, s32 frame, FieldActorState *actor);

/**
 * @brief Copy the rotation terms of one matrix into another.
 * @param dst Matrix receiving the rotation; its translation is unchanged.
 * @param src Matrix supplying the rotation.
 */
void func_800832F0(MATRIX *dst, MATRIX *src)
{
    FieldMatrixRotationWords *to = (FieldMatrixRotationWords *)dst;
    FieldMatrixRotationWords *from = (FieldMatrixRotationWords *)src;

    to->words[0] = from->words[0];
    to->words[1] = from->words[1];
    to->words[2] = from->words[2];
    to->words[3] = from->words[3];
    to->words[4] = from->words[4];
}

/**
 * @brief Advance the texture animation of every mesh part of an actor.
 * @param actor Actor that owns the parts and their frame counters.
 * @param parts Part definitions of @p actor.
 * @param part_count Number of entries in @p parts.
 */
void func_8008332C(FieldActorState *actor, FieldActorPartDef *parts, s32 part_count)
{
    s32 i;

    for (i = 0; i < part_count; i++)
    {
        if ((u8)(parts[i].effect_kind - FIELD_PART_MESH_FIRST) < FIELD_PART_MESH_KIND_COUNT)
        {
            if (parts[i].behavior_flags.word & 1)
            {
                u32 flags = parts[i].track_flags.word;
                func_8008343C(FIELD_PART_MESH_LAST - parts[i].effect_kind, flags & 3, actor, ((flags >> 8) & 7) + 1);
            }
            if ((parts[i].track_flags.word >> 22) & 1)
            {
                if (parts[i].unknown_0x12 != 0 && field_get_track_counter_modulo((struct FieldActorSlot*)actor, parts[i].unknown_0x12) == 0)
                {
                    s32 mesh_index = FIELD_PART_MESH_LAST - parts[i].effect_kind;
                    u8 frame = ++actor->unknown_0x2b[i];
                    func_80083868(mesh_index, frame, actor);
                }
            }
        }
    }
}

/**
 * @brief Scroll every texture part of an actor mesh and upload it to VRAM.
 * @param mesh_index Mesh record in the actor's mesh table.
 * @param direction FIELD_MESH_TEXTURE_SCROLL_* direction.
 * @param actor Actor that owns the mesh; its owner slot selects the VRAM page.
 * @param shift Number of rows or columns moved to the opposite edge.
 * @return Unspecified; no value is returned and callers ignore it.
 */
s32 func_8008343C(s32 mesh_index, s32 direction, FieldActorState *actor, s32 shift)
{
    RECT rect;
    FieldMeshTexturePart *part;
    u16 *scratch;
    u16 *src;
    u16 *dst;
    u16 *row;
    s32 count;
    s32 row_count;
    u8 width;
    s32 part_index;

    scratch = (u16 *)0x1F800000;
    for (part_index = 0; part_index < FIELD_ACTOR_MESH(actor, mesh_index)->texture_part_count; part_index++)
    {
        part = &FIELD_ACTOR_MESH(actor, mesh_index)->texture_parts[part_index];
        switch (direction)
        {
        case FIELD_MESH_TEXTURE_SCROLL_UP:
            src = part->pixels;
            count = part->width * shift;
            dst = src;
            while (count != 0)
            {
                count--;
                *scratch++ = *src++;
            }
            count = part->width * (part->height - shift);
            while (count != 0)
            {
                count--;
                *dst++ = *src++;
            }
            count = part->width * shift;
            scratch = (u16 *)0x1F800000;
            while (count != 0)
            {
                count--;
                *dst++ = *scratch++;
            }
            break;

        case FIELD_MESH_TEXTURE_SCROLL_DOWN:
            width = part->width;
            src = &part->pixels[width * part->height] - 1;
            count = width * shift;
            dst = src;
            while (count != 0)
            {
                count--;
                *scratch++ = *src--;
            }
            count = part->width * (part->height - shift);
            while (count != 0)
            {
                count--;
                *dst-- = *src--;
            }
            count = part->width * shift;
            scratch = (u16 *)0x1F800000;
            while (count != 0)
            {
                count--;
                *dst-- = *scratch++;
            }
            break;

        case FIELD_MESH_TEXTURE_SCROLL_LEFT:
            row_count = part->height;
            src = part->pixels;
            if (row_count != 0)
            {
                do
                {
                    count = 0;
                    if (shift != 0)
                    {
                        dst = scratch;
                        row = src;
                        do
                        {
                            *dst++ = *row++;
                            count++;
                        } while (count != shift);
                    }
                    count = part->width - shift;
                    while (count != 0)
                    {
                        count--;
                        *src = *(src + shift);
                        src++;
                    }
                    count = 0;
                    if (shift != 0)
                    {
                        row = scratch;
                        do
                        {
                            *src++ = *row++;
                            count++;
                        } while (count != shift);
                    }
                    row_count--;
                } while (row_count != 0);
            }
            break;

        case FIELD_MESH_TEXTURE_SCROLL_RIGHT:
            src = &part->pixels[part->width * part->height] - 1;
            row_count = part->height;
            if (row_count != 0)
            {
                do
                {
                    count = 0;
                    if (shift != 0)
                    {
                        dst = scratch;
                        row = src;
                        do
                        {
                            *dst++ = *row--;
                            count++;
                        } while (count != shift);
                    }
                    count = part->width - shift;
                    while (count != 0)
                    {
                        count--;
                        *src = *(src - shift);
                        src--;
                    }
                    count = 0;
                    if (shift != 0)
                    {
                        row = scratch;
                        do
                        {
                            *src-- = *row++;
                            count++;
                        } while (count != shift);
                    }
                    row_count--;
                } while (row_count != 0);
            }
            break;
        }
        if (actor->owner_object_index < 2)
        {
            rect.x = (actor->owner_object_index << 6) + (part->x + FIELD_MESH_TEXTURE_OWNER_VRAM_X);
            rect.y = part->y + FIELD_MESH_TEXTURE_OWNER_VRAM_Y;
        }
        else
        {
            rect.x = part->x + FIELD_MESH_TEXTURE_SHARED_VRAM_X;
            rect.y = part->y;
        }
        rect.w = part->width;
        rect.h = part->height;
        LoadImage(&rect, (u_long *)part->pixels);
    }
}

/**
 * @brief Upload one animation frame of an actor mesh texture to VRAM.
 * @param mesh_index Mesh record in the actor's mesh table.
 * @param frame Frame counter; the frame shown is @p frame modulo the part count.
 * @param actor Actor that owns the mesh; its owner slot selects the VRAM page.
 * @note Every frame is uploaded into the rectangle of the first texture part.
 */
void func_80083868(s32 mesh_index, s32 frame, FieldActorState *actor)
{
    RECT rect;
    FieldMeshTexturePart *base;
    FieldMeshTexturePart *part;

    base = FIELD_ACTOR_MESH(actor, mesh_index)->texture_parts;
    part = &base[frame % (s32)FIELD_ACTOR_MESH(actor, mesh_index)->texture_part_count];
    if (actor->owner_object_index < 2)
    {
        rect.x = (actor->owner_object_index << 6) + (base->x + FIELD_MESH_TEXTURE_OWNER_VRAM_X);
        rect.y = base->y + FIELD_MESH_TEXTURE_OWNER_VRAM_Y;
    }
    else
    {
        rect.x = base->x + FIELD_MESH_TEXTURE_SHARED_VRAM_X;
        rect.y = base->y;
    }
    rect.w = base->width;
    rect.h = base->height;
    LoadImage(&rect, (u_long *)part->pixels);
}
