#include "common.h"

/** @brief Byte view of a field text flags word. */
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

/** @brief Window-geometry table element used by the packed/fixed openers. */
typedef struct
{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
} FieldWindowGeometry;

/** @brief Source window-geometry block read at offset 0x80 when opening a timed window. */
typedef struct
{
    u8 pad0[0x80];
    u16 x;      /* 0x80 */
    u16 y;      /* 0x82 */
    u16 width;  /* 0x84 */
    u16 height; /* 0x86 */
} FieldWindowGeometrySrc;

/** @brief Pending configuration copied into a field text-window state. */
typedef struct
{
    u8 *portrait;               /* 0x00 */
    u16 x;                      /* 0x04 */
    u16 y;                      /* 0x06 */
    u16 width;                  /* 0x08 */
    u16 height;                 /* 0x0A */
    FieldTextAnchorWord anchor; /* 0x0C */
    FieldTextFlags flags;       /* 0x10 */
    u8 *text;                   /* 0x14 */
} FieldTextConfig;

extern u8 *D_801178D0;
extern s32 D_801178D4;
extern u8 g_prim_rect_buf[];

void field_text_open_packed_window();
void field_text_open_fixed_window();
void field_text_start_timed_window(u8 *text);
void func_800674D8(s32 arg0);

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
    extern FieldWindowGeometry D_800EF64C[];

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

/**
 * @brief Set a field text-window string chosen through the offset table.
 * @param slot Window slot index.
 * @param idx String-table index used to pick the window text.
 * @param options Option flags forwarded to field_text_set_string.
 */
void func_8009C77C(s32 slot, s32 idx, s32 options)
{
    field_text_set_string(slot, (u8 *)(D_801178D4 + *(u16 *)((idx * 2) + D_801178D4)), options);
}

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
    extern FieldWindowGeometry D_800EF64C[];

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

/**
 * @brief Thin wrapper forwarding to func_800674D8.
 * @param arg0 Argument passed through unchanged.
 */
void func_8009C954(s32 arg0)
{
    func_800674D8(arg0);
}

/**
 * @brief Reset the pending field text-window config and open a timed window.
 *
 * Clears the config block at 0x801ED408 (portrait, anchor, flag low byte),
 * strips the style/transition flag bits (0x300, 0xC00, 0x7000), copies the
 * window geometry from @c D_800EF64C, then opens a timed text window whose
 * string is selected by @p arg0 through the @c D_801178D4 offset table.
 *
 * @param arg0 String-table index used to pick the window text.
 *
 * @see decomp.me (100%) TODO
 */
void func_8009C974(s32 arg0)
{
    FieldTextConfig *cfg = (FieldTextConfig *)0x801ED408;
    extern FieldWindowGeometrySrc D_800EF64C;

    cfg->flags.b.low = 0;
    cfg->anchor.pos.x = 0;
    cfg->anchor.pos.y = 0;
    cfg->portrait = 0;
    cfg->flags.word &= ~0x300;
    cfg->flags.word &= ~0xC00;
    cfg->flags.word &= ~0x7000;
    cfg->x = D_800EF64C.x;
    cfg->y = D_800EF64C.y;
    cfg->width = D_800EF64C.width;
    cfg->height = D_800EF64C.height;
    field_text_start_timed_window((u8 *)(D_801178D4 + *(u16 *)((arg0 * 2) + D_801178D4)));
}
