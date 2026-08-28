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

/** @brief Source window-geometry block read when opening a text window. */
typedef struct
{
    u8 pad0[0x80];
    u16 x;      /* 0x80 */
    u16 y;      /* 0x82 */
    u16 width;  /* 0x84 */
    u16 height; /* 0x86 */
} FieldWindowGeometry;

extern FieldWindowGeometry D_800EF64C;
extern s32 D_801178D4;

void field_text_start_timed_window(u8 *text);

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
