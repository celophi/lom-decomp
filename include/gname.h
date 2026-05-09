#ifndef _GNAME_H
#define _GNAME_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

/**
 * @brief Name-buffer character encoding.
 *
 * A "name" is a null-terminated byte buffer (`u8*`) used by the name-entry
 * UI. It uses a small DBCS-style variable-width encoding:
 *
 *  - Most bytes are single-byte glyphs (1 byte each).
 *  - A byte in the range [0x19, 0x20) is the *lead* byte of a 2-byte glyph;
 *    the following byte is its trail byte. There are 7 lead-byte values
 *    (0x19..0x1F), giving up to 7 "pages" of wide glyphs.
 *  - 0x00 terminates the string.
 *
 * The `name_*` helpers in gname.c walk the buffer respecting this encoding:
 * `name_byte_length` returns raw bytes, `name_char_count` returns logical
 * glyphs, `name_pop_first_char` / `name_pop_last_char` strip and return one
 * glyph (packing a 2-byte glyph as `lead | (trail << 8)`), and
 * `name_prepend_char` inserts one glyph at the front.
 *
 * `name_is_blank` is a special case: it walks byte-by-byte (not
 * glyph-by-glyph) and treats both ASCII space (0x20) and wide-space
 * sentinel (0x80) as blank.
 */
#define CHAR_SPACE      0x20 /**< ASCII space; blank glyph in name buffers. */
#define CHAR_WIDE_SPACE 0x80 /**< Wide-space sentinel byte; also blank. */

/* True if byte is a custom 2-byte DBCS-style lead byte */
#define IS_DBSC_LEAD_BYTE(c) ((c) >= 0x19 && (c) <= 0x1F)

/* Pack two bytes into a single 16-bit DBCS-style glyph */
#define MAKE_DBCS_GLYPH(lo, hi) (u16)(((u16)(hi) << 8) | (u16)(lo))

/**
 * @brief RGB lerp state.
 *
 * Used as a pair: `g_fade_target` is the *target* (final color + remaining
 * step count), `g_fade_current` is the *current* interpolated value (its
 * `steps` field is unused). Each tick @ref render_fade_overlay advances the
 * current toward the target by `(target - current) / steps` and decrements
 * `steps`. Channels are 0..0x100 with 0x100 meaning "no tint"; values
 * above 0x100 trigger an additive draw mode (GP0 0xE1 abr=2).
 */
typedef struct
{
    s32 r;     /* 0x0 — red channel,   0..0x100 normal, >0x100 = additive */
    s32 g;     /* 0x4 — green channel, 0..0x100 normal, >0x100 = additive */
    s32 b;     /* 0x8 — blue channel,  0..0x100 normal, >0x100 = additive */
    s32 steps; /* 0xC — frames remaining in the lerp (target struct only) */
} FadeState;

// Structure for the argument object
typedef struct
{
    s32 unk0;             // offset 0x00
    char pad[0x4040 - 4]; // padding up to offset 0x4040
    void* unk4040;        // offset 0x4040
} ArgStruct;

typedef struct
{
    u8 _pad0[0x38];
    u32 unk38;
    u8 _pad1[0x4040 - (0x38 + sizeof(u32))];
    s32* unk4040;
    u8 _pad2[0x8];
    u32 unk404C;
} UnkStruct;

/* Object structure (offsets from target assembly) */
typedef struct
{
    u8 _pad0[0x28];
    u32 unk28; /* offset 0x28 */
    u8 _pad1[0x4040 - 0x28 - 4];
    u32* unk4040; /* offset 0x4040 */
    u8 _pad2[0x404C - 0x4040 - 4];
    u32 unk404C; /* offset 0x404C */
} Obj;

/**
 * @brief Glyph metrics entry: how to draw one glyph from VRAM.
 *
 * Used as @c g_glyph_table, indexed by character ID. The
 * fields are written directly into a sprite (tag 0x64) primitive by
 * @ref func_80142274: `u`/`v` at byte offsets 12/13, the CLUT ID at u16
 * offset 14, and `w`/`h` at u16 offsets 16/18.
 */
typedef struct
{
    u8 u;     /* 0x0 — texture U in VRAM */
    u8 v;     /* 0x1 — texture V in VRAM */
    u8 w;     /* 0x2 — sprite width */
    u8 h;     /* 0x3 — sprite height */
    u32 clut; /* 0x4 — CLUT id (low 6 bits used; combined with 0x7C80) */
} GlyphInfo;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad1[2];
} UnkStruct2;

extern void* func_80142274(void* arg0, s32* arg1, u8 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7);

extern s32 D_8014F840;
extern s32 g_name_pixel_width;
extern u8 D_8014F758[];
extern FadeState g_fade_target;
extern FadeState g_fade_current;
extern s32 g_startup_delay;
extern u8 D_80147494[];
extern s32 D_800F22AC;
extern s32 g_strip_width_target;
extern s32 g_strip_width;
extern s32 D_80122988;
extern u8* g_active_name;
extern s32 D_8014F7E4;
extern u8 D_8014F8B8;
extern u8 D_8014F8B0;
extern s8 D_8014F850;
extern char D_8014F7E8;
extern s32 D_8014F848;
extern s32 D_8014F884;
extern s32 D_8014F888;
extern s32 D_8014F88C;
extern s32 D_8014F890;
extern s32 D_8014F894;
extern s32 D_8014F89C;
extern s32 g_strip_width_steps;
extern s32 D_8014F8AC;
extern s32 D_8014F8B4;
extern s32 D_8014F8C0;
extern s32 D_8014F8C4;
extern s32 D_8014F8D0;
extern u8 D_8014F7B0;
extern u8* D_80142F04;
extern s32 D_8014F7E0;
extern s32 D_8014F83C;
extern u8* D_80142F00;
extern u32 D_80142E0C[];
extern s32 D_8014F8A0;
extern u8* D_80142EFC;
extern u32 D_80142E40[];
extern s32 D_8014F898;
extern u8* D_80142EF8;
extern u32 D_80142C98[];
extern u32 D_80142CAC[];
extern s32 D_8014F8C8;
extern s32 D_80142CA4;
extern void* D_8014F84C;
extern s32 D_8014F838;
extern u8 D_80142EF4[];
extern u8 g_glyph_table[]; /* GlyphInfo[]; declared as u8[] for byte-level accesses elsewhere */
extern s32 D_80142E14;
extern u32 D_8014F6B8[];

extern void func_800A3938(int, int);
extern void func_8014139C(void);
extern s32 name_char_count(u8*);
extern s32 name_is_blank(u8*);
extern s32 func_80140AB8(s32, s32);
extern void name_copy(u8*, u8*);

#endif