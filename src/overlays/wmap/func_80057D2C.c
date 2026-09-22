/* Partial WMAP decompilation: 98.924730% (gcc280_g0). */
#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    u8 u, v, width, height;
    u16 palette;
    u8 pad_06[2];
} WmapGlyph;
typedef struct
{
    u16 x;
    u8 y, glyph;
} WmapGlyphPlacement;
typedef struct
{
    u8 pad_00[0x70];
    u32 ordering_table[179];
    u8 *packet_cursor;
} WmapRenderContext;
typedef struct
{
    s32 x, y;
} WmapPosition;
extern WmapGlyph D_800CB2F8[];
extern WmapGlyphPlacement D_800CB3D8[];
extern s32 D_800CB8BC[];
extern s32 D_800D7CE0[];
extern s32 D_800D9160;
extern s32 D_800D921C;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011D4FC;
extern s32 D_8013922C;
extern s32 D_801398C0;
extern s32 D_801398D0;
extern WmapRenderContext *D_801398EC;
extern WmapPosition D_80139950;
extern void func_8005D018(s32, s32, s32, s32 *, s32 *, s32);
extern void func_8006534C(s16, s32);

/** @brief Draw the enabled map information labels and substitute dynamic glyphs. */
void func_80057D2C(void)
{
    s32 dynamic_index;
    s32 group;
    s32 i;
    s32 end;
    s32 next_offset;
    s32 glyph;
    u16 palette;
    WmapGlyphPlacement *placement;
    WmapGlyph *image;
    SPRT *packet;

    if ((D_8013922C & 0xF000) || D_801398D0 != 0)
    {
        D_800D9160 = 0;
    }
    if (!(D_801398C0 & 0x10) || D_8011D4FC == -1)
    {
        D_800D9160 = 0;
        return;
    }
    if (D_800D9160 == 0)
    {
        func_8005D018(D_800DCEEC + D_800DCEF0 * 3, D_80139950.x / 48 + D_800DCEEC,
                     D_80139950.y / 48 + D_800DCEF0, &D_800D9160, D_800D7CE0, D_8011D4FC);
    }
    dynamic_index = 0;
    for (group = 0, next_offset = 4; group < 10; next_offset += 4, group++)
    {
        if ((D_800D9160 >> group) & 1)
        {
            i = D_800CB8BC[group];
            end = *(s32 *)((u8 *)D_800CB8BC + next_offset);
            for (; i < end; i++)
            {
                placement = &D_800CB3D8[i];
                glyph = placement->glyph;
                if (glyph == 255)
                {
                    glyph = D_800D7CE0[dynamic_index++];
                }
                packet = (SPRT *)D_801398EC->packet_cursor;
                packet->x0 = placement->x;
                image = &D_800CB2F8[glyph];
                packet->y0 = placement->y;
                packet->u0 = image->u + 192;
                packet->v0 = image->v + 96;
                packet->w = image->width;
                packet->h = image->height;
                palette = image->palette;
                setlen(packet, 4);
                packet->r0 = 128;
                packet->g0 = 128;
                packet->b0 = 128;
                packet->code = 100;
                packet->clut = ((palette + 432) << 6) | 46;
                addPrim(&D_801398EC->ordering_table[2], packet);
                if (D_800D921C < 32000)
                {
                    D_800D921C += 20;
                    D_801398EC->packet_cursor += 20;
                }
            }
        }
    }
    func_8006534C(59, 2);
}
