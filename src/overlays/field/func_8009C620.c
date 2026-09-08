#include "common.h"

typedef struct
{
    u8 low;
    u8 byte1;
    u8 byte2;
    u8 byte3;
} FieldTextFlagBytes;

typedef union
{
    u32 word;
    FieldTextFlagBytes b;
} FieldTextFlags;

typedef struct
{
    u16 x;
    u16 y;
} FieldTextAnchor;

typedef union
{
    u32 word;
    FieldTextAnchor pos;
} FieldTextAnchorWord;

typedef struct
{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
} FieldWindowGeometry;

typedef struct
{
    u8* portrait;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    FieldTextAnchorWord anchor;
    FieldTextFlags flags;
    u8* text;
} FieldTextConfig;

extern u8* D_801178D0;
extern u8 g_prim_rect_buf[];
extern FieldWindowGeometry D_800EF64C[];

void field_text_open_packed_window();
void field_text_open_fixed_window();

/**
 * @brief Configure and open a field text window.
 * @param arg0 Window slot index.
 * @param arg1 Window layout/style selector.
 * @param arg2 Unused.
 * @param arg3 Portrait/style selector.
 */
void func_8009C620(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    FieldTextConfig* cfg;
    FieldWindowGeometry* geometry;
    FieldWindowGeometry* geometry_table;
    s32 geometry_offset;
    u32 flags;

    cfg = (FieldTextConfig*)0x801ED408;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    if ((arg3 == -1) || ((u32)(arg1 - 2) < 3U))
    {
        cfg->portrait = NULL;
    }
    else if (arg3 & 0x80)
    {
        cfg->portrait = g_prim_rect_buf + (arg3 & 0x3F) * 0x4A0;
    }
    else
    {
        cfg->portrait = D_801178D0 + (arg3 & 0x3F) * 0x4A0;
    }

    cfg->flags.b.low = 0;
    if (arg1 == 6)
    {
        cfg->flags.word = (cfg->flags.word & ~0x300) | 0x100;
    }
    else
    {
        cfg->flags.word &= ~0x300;
    }

    flags = cfg->flags.word;
    geometry_table = D_800EF64C;
    flags &= ~0xC00;
    flags |= (arg3 << 4) & 0x400;
    flags &= ~0x7000;
    geometry_offset = arg0 << 3;
    cfg->flags.word = flags;
    geometry = (FieldWindowGeometry*)((u8*)geometry_table + (geometry_offset + (arg1 << 5)));
    cfg->x = geometry->x;
    cfg->y = geometry->y;
    cfg->width = geometry->width;
    cfg->height = geometry->height;

    if (arg1 >= 5)
    {
        field_text_open_packed_window(arg0);
        return;
    }
    field_text_open_fixed_window(arg0);
}
