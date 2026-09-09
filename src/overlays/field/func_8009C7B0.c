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
extern s32 D_801178D4;
extern u8 g_prim_rect_buf[];
extern FieldWindowGeometry D_800EF64C[];

void field_text_open_packed_window();
void field_text_open_fixed_window();

/**
 * @brief Configure, open, and populate a field text window.
 * @param string_index String index in the field text table.
 * @param window_slot Window slot index.
 * @param layout_index Window layout/style selector.
 * @param unused Unused.
 * @param portrait_selector Portrait/style selector.
 */
void func_8009C7B0(s32 string_index, s32 window_slot, s32 layout_index, s32 unused, s32 portrait_selector)
{
    FieldTextConfig* cfg;
    FieldWindowGeometry* geometry;
    FieldWindowGeometry* geometry_table;
    s32 geometry_offset;
    u32 flags;

    cfg = (FieldTextConfig*)0x801ED408;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    if ((portrait_selector == -1) || ((u32)(layout_index - 2) < 3U))
    {
        cfg->portrait = NULL;
    }
    else if (portrait_selector & 0x80)
    {
        cfg->portrait = g_prim_rect_buf + (portrait_selector & 0x3F) * 0x4A0;
    }
    else
    {
        cfg->portrait = D_801178D0 + portrait_selector * 0x12360;
    }

    cfg->flags.b.low = 0;
    if (layout_index == 6)
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
    flags |= (portrait_selector << 4) & 0x400;
    flags &= ~0x7000;
    geometry_offset = window_slot << 3;
    cfg->flags.word = flags;
    geometry = (FieldWindowGeometry*)((u8*)geometry_table + (geometry_offset + (layout_index << 5)));
    cfg->x = geometry->x;
    cfg->y = geometry->y;
    cfg->width = geometry->width;
    cfg->height = geometry->height;

    if (layout_index >= 5)
    {
        field_text_open_packed_window(window_slot);
    }
    else
    {
        field_text_open_fixed_window(window_slot);
    }

    field_text_set_string(window_slot, (u8*)(D_801178D4 + *(u16*)((string_index * 2) + D_801178D4)), 1);
}
