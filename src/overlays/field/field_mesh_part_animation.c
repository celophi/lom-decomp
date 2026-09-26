/** @file field_mesh_part_animation.c
 * @brief Copy matrix rotations and animate the VRAM texture parts of actor meshes.
 *
 * A mesh texture part is a small 16-bit image kept in main memory. Scrolling
 * rotates its rows or columns in place (through the scratchpad) and uploads
 * it again; frame animation uploads one of the parts into the rectangle of
 * the first part.
 */

#include "common.h"
#include "field_effect_types.h"
#include "field_actor_runtime.h"
#include "field_mesh.h"
#include "field_mesh_transform.h"
#include "field_actor_palette.h"
#include "sdk/libetc.h"
#include "sdk/libgpu.h"

/** @brief First part effect kind that renders a mesh; kinds up to FIELD_PART_MESH_LAST select meshes 2..0. */
#define FIELD_PART_MESH_FIRST 0xF7
/** @brief Part effect kind that renders mesh 0. */
#define FIELD_PART_MESH_LAST 0xF9

/** @brief FieldActorPartDef behavior_flags bit: the mesh texture scrolls. */
#define FIELD_PART_TEXTURE_SCROLLS 0x1
/** @brief FieldActorPartDef track_flags fields of the texture animation. */
#define FIELD_PART_SCROLL_DIRECTION(flags) ((flags) & 3)
#define FIELD_PART_SCROLL_SPEED(flags) ((((flags) >> 8) & 7) + 1)
#define FIELD_PART_TEXTURE_FRAMES(flags) (((flags) >> 22) & 1)

/** @brief Texture scroll directions (FIELD_PART_SCROLL_DIRECTION). */
enum
{
    FIELD_MESH_TEXTURE_SCROLL_UP = 0,
    FIELD_MESH_TEXTURE_SCROLL_DOWN = 1,
    FIELD_MESH_TEXTURE_SCROLL_LEFT = 2,
    FIELD_MESH_TEXTURE_SCROLL_RIGHT = 3
};

/** @brief Rotation part of a MATRIX (the 3x3 terms and padding) as five words. */
typedef struct
{
    s32 words[5];
} MatrixRotationWords;

static s32 field_scroll_mesh_texture(s32 mesh_index, s32 direction, FieldActorState* actor, s32 shift);
static void field_show_mesh_texture_frame(s32 mesh_index, s32 frame, FieldActorState* actor);

/**
 * @brief Copy the rotation terms of one matrix into another.
 * @param dst Matrix receiving the rotation; its translation is unchanged.
 * @param src Matrix supplying the rotation.
 */
void field_copy_matrix_rotation(MATRIX* dst, MATRIX* src)
{
    MatrixRotationWords* to = (MatrixRotationWords*)dst->m;
    MatrixRotationWords* from = (MatrixRotationWords*)src->m;

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
void field_animate_mesh_textures(FieldActorState* actor, FieldActorPartDef* parts, s32 part_count)
{
    s32 i;

    for (i = 0; i < part_count; i++)
    {
        if (parts[i].effect_kind >= FIELD_PART_MESH_FIRST && parts[i].effect_kind <= FIELD_PART_MESH_LAST)
        {
            if (parts[i].behavior_flags.word & FIELD_PART_TEXTURE_SCROLLS)
            {
                u32 flags = parts[i].track_flags.word;

                field_scroll_mesh_texture(FIELD_PART_MESH_LAST - parts[i].effect_kind, FIELD_PART_SCROLL_DIRECTION(flags), actor,
                                          FIELD_PART_SCROLL_SPEED(flags));
            }
            if (FIELD_PART_TEXTURE_FRAMES(parts[i].track_flags.word))
            {
                if (parts[i].texture_frame_period != 0 &&
                    field_get_track_counter_modulo((struct FieldActorSlot*)actor, parts[i].texture_frame_period) == 0)
                {
                    s32 mesh_index = FIELD_PART_MESH_LAST - parts[i].effect_kind;
                    u8 frame = ++actor->mesh_texture_frames[i];

                    field_show_mesh_texture_frame(mesh_index, frame, actor);
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
 * @return Nothing meaningful; callers ignore it.
 * @note Declared int without a return statement: as void, v0 is dead at the
 *       exit and the loop branch gets a different delay slot.
 */
static s32 field_scroll_mesh_texture(s32 mesh_index, s32 direction, FieldActorState* actor, s32 shift)
{
    RECT rect;
    FieldMeshTexturePart* part;
    u16* scratch;
    u16* src;
    u16* dst;
    u16* row;
    s32 count;
    s32 row_count;
    u8 width;
    s32 part_index;

    scratch = (u16*)getScratchAddr(0);
    for (part_index = 0; part_index < FIELD_ACTOR_MESH(actor, mesh_index)->texture_part_count; part_index++)
    {
        part = &FIELD_ACTOR_MESH(actor, mesh_index)->texture_parts[part_index];
        switch (direction)
        {
        case FIELD_MESH_TEXTURE_SCROLL_UP:
            /* Save the top rows, move the rest up and put the saved rows at the bottom. */
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
            scratch = (u16*)getScratchAddr(0);
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
            scratch = (u16*)getScratchAddr(0);
            while (count != 0)
            {
                count--;
                *dst-- = *scratch++;
            }
            break;

        case FIELD_MESH_TEXTURE_SCROLL_LEFT:
            /* Per row: save the left columns, move the rest left and put the saved columns at the right. */
            row_count = part->height;
            src = part->pixels;
            while (row_count != 0)
            {
                for (count = 0; count != shift; count++)
                {
                    scratch[count] = src[count];
                }
                count = part->width - shift;
                while (count != 0)
                {
                    count--;
                    *src = *(src + shift);
                    src++;
                }
                for (count = 0; count != shift; count++)
                {
                    *src++ = scratch[count];
                }
                row_count--;
            }
            break;

        case FIELD_MESH_TEXTURE_SCROLL_RIGHT:
            src = &part->pixels[part->width * part->height] - 1;
            row_count = part->height;
            while (row_count != 0)
            {
                for (count = 0; count != shift; count++)
                {
                    scratch[count] = src[-count];
                }
                count = part->width - shift;
                while (count != 0)
                {
                    count--;
                    *src = *(src - shift);
                    src--;
                }
                for (count = 0; count != shift; count++)
                {
                    *src-- = scratch[count];
                }
                row_count--;
            }
            break;
        }
        if (actor->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)
        {
            rect.x = actor->owner_object_index * FIELD_ACTOR_TEXTURE_VRAM_WIDTH + (part->x + FIELD_ACTOR_TEXTURE_VRAM_X);
            rect.y = part->y + FIELD_ACTOR_TEXTURE_VRAM_Y;
        }
        else
        {
            rect.x = part->x + FIELD_SHARED_TEXTURE_VRAM_X;
            rect.y = part->y + FIELD_SHARED_TEXTURE_VRAM_Y;
        }
        rect.w = part->width;
        rect.h = part->height;
        LoadImage(&rect, (u_long*)part->pixels);
    }
}

/**
 * @brief Upload one animation frame of an actor mesh texture to VRAM.
 * @param mesh_index Mesh record in the actor's mesh table.
 * @param frame Frame counter; the frame shown is @p frame modulo the part count.
 * @param actor Actor that owns the mesh; its owner slot selects the VRAM page.
 * @note Every frame is uploaded into the rectangle of the first texture part.
 */
static void field_show_mesh_texture_frame(s32 mesh_index, s32 frame, FieldActorState* actor)
{
    RECT rect;
    FieldMeshTexturePart* base;
    FieldMeshTexturePart* part;

    base = FIELD_ACTOR_MESH(actor, mesh_index)->texture_parts;
    part = &base[frame % (s32)FIELD_ACTOR_MESH(actor, mesh_index)->texture_part_count];
    if (actor->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)
    {
        rect.x = actor->owner_object_index * FIELD_ACTOR_TEXTURE_VRAM_WIDTH + (base->x + FIELD_ACTOR_TEXTURE_VRAM_X);
        rect.y = base->y + FIELD_ACTOR_TEXTURE_VRAM_Y;
    }
    else
    {
        rect.x = base->x + FIELD_SHARED_TEXTURE_VRAM_X;
        rect.y = base->y + FIELD_SHARED_TEXTURE_VRAM_Y;
    }
    rect.w = base->width;
    rect.h = base->height;
    LoadImage(&rect, (u_long*)part->pixels);
}
