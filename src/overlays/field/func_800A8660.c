#include "common.h"

/** @brief Eight-word animation lookup table copied to local storage. */
typedef struct
{
    s32 words[8];
} Table;
/** @brief Textured four-point GPU primitive with packed tag and color words. */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u8 link[3];
            u8 len;
        } bytes;
    } tag;
    union
    {
        u32 word;
        struct
        {
            u8 r, g, b, code;
        } bytes;
    } color;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad3;
} Quad;
extern Table D_800513E8, D_80051408, D_80051428, D_80051448;
extern s32 g_frame_counter;
/**
 * @brief Append an animated textured quad to an ordering table.
 * @param arg0 Ordering-table entry receiving the primitive.
 * @param arg1 Writable primitive buffer.
 * @param arg2 Horizontal origin.
 * @param arg3 Vertical origin.
 * @return Buffer address immediately after the emitted primitive.
 */
void *func_800A8660(s32 *arg0, Quad *arg1, s32 arg2, s32 arg3)
{
    s32 frame;
    u32 color;
    Table tables[4];
    s32 temp_a3;
    u16 temp_v0;
    u16 temp_v0_2;
    s32 temp_a2_2;
    s32 temp_a2;
    u8 *temp_v1;

    tables[0] = D_800513E8;
    tables[1] = D_80051408;
    tables[2] = D_80051428;
    tables[3] = D_80051448;
    color = 0x808080;
    frame = g_frame_counter;
    arg1->tag.bytes.len = 9;
    arg1->color.word = color;
    arg1->color.bytes.code = 0x2C;
    temp_v1 = ((((frame >> 2) + 3) & 7) * 4) + (u8 *)tables;
    temp_v0 = *(u16 *)(temp_v1 + 0x20) + arg2;
    arg1->x2 = temp_v0;
    arg1->x0 = temp_v0;
    arg2 = *(u16 *)(temp_v1 + 0x60);
    arg1->y1 = arg3;
    arg1->y0 = arg3;
    temp_a3 = arg3 + 0x10;
    arg1->y3 = temp_a3;
    arg1->y2 = temp_a3;
    temp_v0_2 = temp_v0 + arg2;
    arg1->x3 = temp_v0_2;
    arg1->x1 = temp_v0_2;
    arg2 = *(u8 *)(temp_v1 + 0x40);
    arg1->u2 = arg2;
    arg1->u0 = arg2;
    arg1->v1 = 0x80;
    arg1->v0 = 0x80;
    arg1->v3 = 0x90;
    arg1->v2 = 0x90;
    arg1->clut = 0x7A87;
    arg2 += *(u8 *)(temp_v1 + 0x60);
    arg1->u3 = arg2;
    arg1->u1 = arg2;
    arg1->tpage = 0x26;
    arg1->tag.word = (s32)((arg1->tag.word & 0xFF000000) | (*arg0 & 0xFFFFFF));
    *arg0 = (*arg0 & 0xFF000000) | ((s32)arg1 & 0xFFFFFF);
    return arg1 + 1;
}
