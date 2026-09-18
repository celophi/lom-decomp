/** @file field_mesh_part_animation.c
 * @brief Copy mesh state and update the animated sprite parts used by the mesh renderer.
 */

#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
} UnkStruct14;

void func_800832F0(UnkStruct14 *arg0, UnkStruct14 *arg1)
{
    arg0->unk0 = arg1->unk0;
    arg0->unk4 = arg1->unk4;
    arg0->unk8 = arg1->unk8;
    arg0->unkC = arg1->unkC;
    arg0->unk10 = arg1->unk10;
}

typedef struct
{
    u32 unk0;         /* 0x00 */
    s32 unk4;         /* 0x04 */
    u8  unk8[3];      /* 0x08 */
    u8  unkB;         /* 0x0B */
    u8  unkC[6];      /* 0x0C */
    u8  unk12;        /* 0x12 */
    u8  unk13[0x35];  /* 0x13, stride 0x48 */
} UnkPartEntry;

/**
 * @see decomp.me (100%) local match - no scratch link created.
 */
void func_8008332C(u8 *arg0, UnkPartEntry *arg1, s32 arg2)
{
    s32 var_s1;
    u32 temp_a3;
    u8 temp_v0_2;
    u8 temp_test;
    s32 temp_a0;

    for (var_s1 = 0; var_s1 < arg2; var_s1++)
    {
        temp_test = (arg1[var_s1].unkB + 9) & 0xFF;
        if ((u32) temp_test < 3U)
        {
            if (arg1[var_s1].unk4 & 1)
            {
                temp_a3 = arg1[var_s1].unk0;
                func_8008343C(0xF9 - arg1[var_s1].unkB, temp_a3 & 3, arg0, ((temp_a3 >> 8) & 7) + 1);
            }
            if (((u32) arg1[var_s1].unk0 >> 0x16) & 1)
            {
                if ((arg1[var_s1].unk12 != 0) && (field_get_track_counter_modulo(arg0, arg1[var_s1].unk12) == 0))
                {
                    temp_a0 = 0xF9 - arg1[var_s1].unkB;
                    temp_v0_2 = (arg0 + var_s1)[0x2B] + 1;
                    (arg0 + var_s1)[0x2B] = temp_v0_2;
                    func_80083868(temp_a0, temp_v0_2 & 0xFF, arg0);
                }
            }
        }
    }
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;   /* width */
    u8 unk3;   /* height */
    u16 *unk4; /* pixel data */
} PartEntry;   /* 0x8 */

typedef struct
{
    u8 pad0[2];
    u8 unk2;          /* part count */
    u8 pad3;
    PartEntry *unk4;  /* parts array */
    u8 pad8[0x18 - 0x8];
} ActorEntry;         /* 0x18 */

typedef struct
{
    u8 pad0[0x18];
    ActorEntry *unk18;
    u8 pad1C[0x228 - 0x1C];
    u8 unk228;
} FieldCtx;

/**
 * @brief Rotate or shift each mesh part and upload the updated pixels to VRAM.
 * @param actor_index Actor index in the field actor array.
 * @param mode Pixel rotation/shift mode in the range 0 through 3.
 * @param field_ctx Field context containing the actor array and VRAM page selector.
 * @param shift_count Number of rows or columns moved to the opposite edge.
 * @return Zero after all parts have been processed.
 */
s32 func_8008343C(s32 actor_index, s32 mode, FieldCtx *field_ctx, s32 shift_count)
{
    RECT rect;
    ActorEntry *actor;
    PartEntry *part;
    u16 *scratch;
    u16 *cursor_a;
    u16 *cursor_b;
    u16 *cursor_c;
    s32 count;
    u16 pixel;
    u8 width;
    s32 row_count;
    s32 row_stride;
    s32 part_index;
    ActorEntry *actors;
    s32 actor_index_x2;
    s32 actor_index_x3;
    s32 result;

    scratch = (u16 *)0x1F800000;
    actor_index_x2 = actor_index * 2;
    actor_index_x3 = actor_index_x2 + actor_index;
    do
    {
        part_index = 0;
    } while (0);
    actors = field_ctx->unk18;
    actor = (ActorEntry *)((actor_index_x3 * 8) + (s32)actors);
    result = actor->unk2;
    if (result != 0)
    {
        row_stride = shift_count * 2;
        do
        {
            actor = (ActorEntry *)((((actor_index_x2 + actor_index) * 8) + (s32)actors));
            part = &actor->unk4[part_index];
            switch (mode)
            {
            case 0:
                cursor_a = part->unk4;
                count = part->unk2 * shift_count;
                cursor_b = cursor_a;
                if (count != 0)
                {
                    do
                    {
                        count -= 1;
                        pixel = *cursor_a;
                        cursor_a += 1;
                        *scratch = pixel;
                        scratch += 1;
                    } while (count != 0);
                }
                count = part->unk2 * (part->unk3 - shift_count);
                if (count != 0)
                {
                    do
                    {
                        count -= 1;
                        pixel = *cursor_a;
                        cursor_a += 1;
                        *cursor_b = pixel;
                        cursor_b += 1;
                    } while (count != 0);
                }
                count = part->unk2 * shift_count;
                scratch = (u16 *)0x1F800000;
                if (count != 0)
                {
                    do
                    {
                        count -= 1;
                        pixel = *scratch;
                        scratch += 1;
                        *cursor_b = pixel;
                        cursor_b += 1;
                    } while (count != 0);
                }
                break;
            case 1:
                width = part->unk2;
                cursor_a = &part->unk4[width * part->unk3] - 1;
                count = width * shift_count;
                cursor_b = cursor_a;
                if (count != 0)
                {
                    do
                    {
                        count -= 1;
                        pixel = *cursor_a;
                        cursor_a -= 1;
                        *scratch = pixel;
                        scratch += 1;
                    } while (count != 0);
                }
                count = part->unk2 * (part->unk3 - shift_count);
                if (count != 0)
                {
                    do
                    {
                        count -= 1;
                        pixel = *cursor_a;
                        cursor_a -= 1;
                        *cursor_b = pixel;
                        cursor_b -= 1;
                    } while (count != 0);
                }
                count = part->unk2 * shift_count;
                scratch = (u16 *)0x1F800000;
                if (count != 0)
                {
                    do
                    {
                        count -= 1;
                        pixel = *scratch;
                        scratch += 1;
                        *cursor_b = pixel;
                        cursor_b -= 1;
                    } while (count != 0);
                }
                break;
            case 2:
                row_count = part->unk3;
                cursor_a = part->unk4;
                if (row_count != 0)
                {
                    do
                    {
                        count = 0;
                        if (shift_count != 0)
                        {
                            cursor_b = scratch;
                            cursor_c = cursor_a;
                            do
                            {
                                pixel = *cursor_c;
                                cursor_c += 1;
                                count += 1;
                                *cursor_b = pixel;
                                cursor_b += 1;
                            } while (count != shift_count);
                        }
                        count = part->unk2 - shift_count;
                        if (count != 0)
                        {
                            do
                            {
                                count -= 1;
                                *cursor_a = *(u16 *)(row_stride + (s32)cursor_a);
                                cursor_a += 1;
                            } while (count != 0);
                        }
                        count = 0;
                        if (shift_count != 0)
                        {
                            cursor_c = scratch;
                            do
                            {
                                pixel = *cursor_c;
                                cursor_c += 1;
                                count += 1;
                                *cursor_a = pixel;
                                cursor_a += 1;
                            } while (count != shift_count);
                        }
                        row_count -= 1;
                    } while (row_count != 0);
                }
                break;
            case 3:
                cursor_a = &part->unk4[part->unk2 * part->unk3] - 1;
                row_count = part->unk3;
                if (row_count != 0)
                {
                    do
                    {
                        count = 0;
                        if (shift_count != 0)
                        {
                            cursor_b = scratch;
                            cursor_c = cursor_a;
                            do
                            {
                                pixel = *cursor_c;
                                cursor_c -= 1;
                                count += 1;
                                *cursor_b = pixel;
                                cursor_b += 1;
                            } while (count != shift_count);
                        }
                        count = part->unk2 - shift_count;
                        if (count != 0)
                        {
                            do
                            {
                                count -= 1;
                                *cursor_a = *(u16 *)((u8 *)cursor_a - row_stride);
                                cursor_a -= 1;
                            } while (count != 0);
                        }
                        count = 0;
                        if (shift_count != 0)
                        {
                            cursor_c = scratch;
                            do
                            {
                                pixel = *cursor_c;
                                cursor_c += 1;
                                count += 1;
                                *cursor_a = pixel;
                                cursor_a -= 1;
                            } while (count != shift_count);
                        }
                        row_count -= 1;
                    } while (row_count != 0);
                }
                break;
            }
            if ((u8) field_ctx->unk228 < 2U)
            {
                rect.x = (field_ctx->unk228 << 6) + (part->unk0 + 0x340);
                rect.y = part->unk1 + 0x100;
            }
            else
            {
                rect.x = part->unk0 + 0x140;
                rect.y = part->unk1;
            }
            rect.w = part->unk2;
            rect.h = part->unk3;
            LoadImage(&rect, part->unk4);
            actor_index_x2 = actor_index * 2;
            actors = field_ctx->unk18;
            part_index += 1;
            actor = (ActorEntry *)((((actor_index_x2 + actor_index) * 8) + (s32)actors));
            result = part_index < (s32)actor->unk2;
        } while (result != 0);
    }
    return result;
}

/**
 * @brief Reload one part of an actor sprite (selected by arg1 modulo the part
 *        count) into VRAM via LoadImage, using the base part's RECT dimensions.
 * @param arg0 Actor index into arg2->unk18[].
 * @param arg1 Part selector; the part used is arg1 % actor->unk2.
 * @param arg2 Field context; unk18 is the actor array, unk228 selects the VRAM
 *             destination page.
 * @see decomp.me (100%) local match - no scratch link created.
 */
void func_80083868(s32 arg0, s32 arg1, FieldCtx *arg2)
{
    RECT rect;
    PartEntry *base;
    PartEntry *part;

    base = arg2->unk18[arg0].unk4;
    part = &base[arg1 % (s32) arg2->unk18[arg0].unk2];
    if ((u8) arg2->unk228 < 2U)
    {
        rect.x = (arg2->unk228 << 6) + (base->unk0 + 0x340);
        rect.y = base->unk1 + 0x100;
    }
    else
    {
        rect.x = base->unk0 + 0x140;
        rect.y = base->unk1;
    }
    rect.w = base->unk2;
    rect.h = base->unk3;
    LoadImage(&rect, part->unk4);
}
