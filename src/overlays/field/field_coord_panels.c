#include "common.h"

/*
 * Modal coordinate panels: func_800AB86C draws the panel for pad-context slot 1
 * and func_800AC768 the panel for the slot selected by D_80122820.  Each shows
 * a position label plus formatted coordinate pairs pulled from the pad context.  The string
 * table entries (D_800EC3C4-relative, little-endian u16 offsets read as two
 * bytes) supply the minus sign and the separator glyphs.
 */

/** @brief Two-byte little-endian offset entry of the D_800EC3C4 string table. */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

/** @brief Caller-owned block whose draw handle lives at 0x40B8. */
typedef struct
{
    u8 pad0[0x40B8];
    s32 unk40B8;
} ArgA;

extern StructEC D_800EC3E4;
extern StructEC D_800EC406;
extern StructEC D_800EC408;
extern StructEC D_800EC40A;
extern StructEC D_800EC40C;
extern s32 D_8011F3D4;
extern s32 D_80122820;
extern s32 D_80122990;
extern s32 D_80122B08;
extern u8 *g_pad_ctx;

void func_800A3938(s32 sound_id, s32 pan);
s32 func_800A8DDC(u8 *arg0);
void func_800A8E28(u8 *dest, u8 *src);
s32 func_800AEAC0(s32 handle, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
s32 func_800AF950(s32 handle, void *arg1, u8 *str, s32 arg3, s32 x, s32 y, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11);

/** @brief Resolve a string-table entry: table base plus its 16-bit offset. */
#define STR_TABLE_ENTRY(sym, off) ((u8 *)(((sym).unk1 << 8) + (sym).unk0 + ((u8 *)&(sym) - (off))))

/** @brief True for a DBCS lead byte (0x19-0x1F), which owns the following byte. */
#define IS_DBCS(c) ((u32)((c) - 0x19) < 7)

/** @brief Write a signed decimal into @p buf (minus-sign glyph from the string table). */
#define FORMAT_SIGNED(buf, val)                                       \
    {                                                                 \
    u8 *dst;                                                          \
    s32 value;                                                        \
    s32 wide;                                                         \
    u8 *minus;                                                        \
    s32 low;                                                          \
    s32 offset;                                                       \
    s32 div;                                                          \
    s32 started;                                                      \
    s32 digit;                                                        \
    dst = buf;                                                        \
    value = val;                                                      \
    wide = 0;                                                         \
    if (value < 0)                                                    \
    {                                                                 \
        value = -value;                                               \
        low = D_800EC3E4.unk0;                                        \
        offset = (D_800EC3E4.unk1 << 8) + (s32)((u8 *)&D_800EC3E4 - 0x20); \
        minus = (u8 *)(low + offset);                                 \
        func_800A8E28(dst, minus);                                    \
        dst += func_800A8DDC(minus);                                  \
    }                                                                 \
    div = 10000000;                                                   \
    started = 0;                                                      \
    do                                                                \
    {                                                                 \
        digit = value / div;                                          \
        if (digit != 0)                                               \
        {                                                             \
            started = 1;                                              \
        }                                                             \
        if (started || div == 1)                                      \
        {                                                             \
            if (wide)                                                 \
            {                                                         \
                *dst++ = 0x1D;                                        \
                *dst = digit;                                         \
            }                                                         \
            else                                                      \
            {                                                         \
                *dst = digit + '0';                                   \
            }                                                         \
            dst++;                                                    \
            value -= (value / div) * div;                             \
        }                                                             \
        div /= 10;                                                    \
    } while (div != 0);                                               \
    *dst = 0;                                                         \
    }

/** @brief Advance @p p to the terminator, accumulating the DBCS-aware byte length. */
#define STR_LEN_LOOP(p, len)                                          \
    while (*p != 0)                                                   \
    {                                                                 \
        if (IS_DBCS(*p))                                              \
        {                                                             \
            p += 2;                                                   \
            len += 2;                                                 \
        }                                                             \
        else                                                          \
        {                                                             \
            p += 1;                                                   \
            len += 1;                                                 \
        }                                                             \
    }

/** @brief Shared strcat body: append @p s_ after the last glyph of @p d_. */
#define STR_CAT_BODY(d_, s_, qsrc)                                    \
        volatile u8 *p = d_;                                          \
        s32 len_d = 0;                                                \
        volatile u8 *q;                                               \
        s32 len_s;                                                    \
        s32 append;                                                   \
        s32 i;                                                        \
        STR_LEN_LOOP(p, len_d)                                        \
        q = qsrc;                                                     \
        len_s = 0;                                                    \
        append = len_d;                                               \
        STR_LEN_LOOP(q, len_s)                                        \
        for (i = 0; i < len_s; i++)                                   \
        {                                                             \
            d_[i + append] = s_[i];                                   \
        }                                                             \
        d_[i + append] = 0;

/** @brief Append buffer @p s onto @p d. */
#define STR_CAT(d, s)                                                 \
    {                                                                 \
        u8 *d_ = (d);                                                 \
        u8 *s_ = (s);                                                 \
        STR_CAT_BODY(d_, s_, (s))                                     \
    }

/** @brief Append string-table entry @p sym onto @p d. */
#define STR_CAT_ENTRY(d, sym, off)                                    \
    {                                                                 \
        u8 *d_ = (d);                                                 \
        u8 *s_ = STR_TABLE_ENTRY(sym, off);                                \
        STR_CAT_BODY(d_, s_, s_)                                      \
    }

/** @brief Panel contents are drawn only while the fade state is 0 or 2. */
#define DRAW_FLAG (D_8011F3D4 == 0 || D_8011F3D4 == 2)

/**
 * @brief Step the modal result panel and queue its draw primitives.
 *
 * Advances the D_8011F3D4 fade state machine (halve the slide offset, hold for
 * 0x5A frames, then slide back out), then builds and queues the panel frame,
 * the position label, the formatted "x,y" coordinate string and, while the
 * pad-context mode byte at 0x858 is below 2, a second coordinate string.
 *
 * @param arg0 Block holding the draw handle at 0x40B8.
 * @return 1 once the panel has fully slid out (state 3), otherwise 0.
 * @note 99.92% (943/959 rows): the only residue is the register holding the
 *       16-bit offset sum inside STR_CAT_ENTRY (target lands it in the entry
 *       pointer's register, ours in a temp) at the four call sites.
 * @see decomp.me (99.92%) TODO
 */
s32 func_800AB86C(ArgA *arg0)
{
    u8 buf[0x38];
    u8 buf2[0x38];
    s32 handle;
    void *ctx;

    handle = arg0->unk40B8;
    ctx = arg0;
    switch (D_8011F3D4)
    {
    case 0:
        D_80122990 /= 2;
        if (D_80122990 == 0)
        {
            D_8011F3D4 = 1;
            D_80122B08 = 0x5A;
        }
        break;
    case 1:
        D_80122B08--;
        if (D_80122B08 == 0)
        {
            func_800A3938(0x127, 0x80);
            D_8011F3D4 = 2;
            D_80122990 = -1;
        }
        break;
    case 2:
        D_80122990 *= 2;
        if (D_80122990 < -0x64)
        {
            D_8011F3D4 = 3;
        }
        break;
    case 3:
        return 1;
    }

    handle = func_800AEAC0(handle, ctx, 0, D_80122990 + 0x32, 0x22, 1);
    handle = func_800AF950(handle, ctx, g_pad_ctx + 0x5F0, 4, D_80122990 + 0x6C, 0x32, 0, 5, 0x180, 0x180, -4, DRAW_FLAG);

    FORMAT_SIGNED(buf, *(u16 *)(g_pad_ctx + 0x634));
    STR_CAT_ENTRY(buf, D_800EC408, 0x44);
    FORMAT_SIGNED(buf2, *(u16 *)(g_pad_ctx + 0x636));
    STR_CAT(buf, buf2);
    STR_CAT_ENTRY(buf, D_800EC40A, 0x46);

    handle = func_800AF950(handle, ctx, buf, 4, D_80122990 + 0x6C, 0x42, 0, 6, 0x180, 0x180, -4, DRAW_FLAG);
    {
        s32 low = D_800EC406.unk0;
        s32 offset = (D_800EC406.unk1 << 8) + (s32)((u8 *)&D_800EC406 - 0x42);
        handle = func_800AF950(handle, ctx, (u8 *)(low + offset), 4, 0xA0, 0x64, 2, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }
    handle = func_800AEAC0(handle, ctx, 1, 0xDE - D_80122990, 0x86, 0);
    handle = func_800AF950(handle, ctx, g_pad_ctx + 0x840, 4, 0xD4 - D_80122990, 0x96, 1, 8, 0x180, 0x180, -4, DRAW_FLAG);

    if ((u32)(g_pad_ctx[0x858] & 0x7F) < 2)
    {
        FORMAT_SIGNED(buf, *(u16 *)(g_pad_ctx + 0x884));
        STR_CAT_ENTRY(buf, D_800EC408, 0x44);
        FORMAT_SIGNED(buf2, *(u16 *)(g_pad_ctx + 0x886));
        STR_CAT(buf, buf2);
        STR_CAT_ENTRY(buf, D_800EC40A, 0x46);
        handle = func_800AF950(handle, ctx, buf, 4, 0xD4 - D_80122990, 0xA6, 1, 9, 0x180, 0x180, -4, DRAW_FLAG);
    }

    arg0->unk40B8 = handle;
    return 0;
}

/**
 * @brief Step the selected-slot coordinate panel and queue its draw primitives.
 *
 * Same fade state machine and layout as func_800AB86C, but the pad-context
 * record is chosen by D_80122820 (stride 0x250) and the panel title comes from
 * string-table entry 0x48.
 *
 * @param arg0 Block holding the draw handle at 0x40B8.
 * @return 1 once the panel has fully slid out (state 3), otherwise 0.
 * @note 99.91% (552/562 rows): the STR_CAT_ENTRY offset-sum register residue
 *       (see func_800AB86C) at two sites, plus one register pick for the
 *       second record-address computation.
 * @see decomp.me (99.91%) TODO
 */
s32 func_800AC768(ArgA *arg0)
{
    u8 buf[0x38];
    u8 buf2[0x38];
    s32 handle;
    void *ctx;
    u8 *rec;

    handle = arg0->unk40B8;
    ctx = arg0;
    switch (D_8011F3D4)
    {
    case 0:
        D_80122990 /= 2;
        if (D_80122990 == 0)
        {
            D_8011F3D4 = 1;
            D_80122B08 = 0x5A;
        }
        break;
    case 1:
        D_80122B08--;
        if (D_80122B08 == 0)
        {
            func_800A3938(0x127, 0x80);
            D_8011F3D4 = 2;
            D_80122990 = -1;
        }
        break;
    case 2:
        D_80122990 *= 2;
        if (D_80122990 < -0x64)
        {
            D_8011F3D4 = 3;
        }
        break;
    case 3:
        return 1;
    }

    {
        s32 low = D_800EC40C.unk0;
        s32 offset = (D_800EC40C.unk1 << 8) + (s32)((u8 *)&D_800EC40C - 0x48);
        handle = func_800AF950(handle, ctx, (u8 *)(low + offset), 4, 0xA0, 0x34, 2, 5, 0x200, 0x200, -4, DRAW_FLAG);
    }
    handle = func_800AEAC0(handle, ctx, D_80122820, D_80122990 + 0x32, 0x54, 1);
    handle = func_800AF950(handle, ctx, g_pad_ctx + (D_80122820 * 0x250 + 0x5F0), 4, D_80122990 + 0x6C, 0x64, 0, 6, 0x1C0, 0x1C0, -4, DRAW_FLAG);

    rec = g_pad_ctx + D_80122820 * 0x250;
    if ((u32)(rec[0x608] & 0x7F) < 2)
    {
        FORMAT_SIGNED(buf, *(u16 *)(rec + 0x634));
        STR_CAT_ENTRY(buf, D_800EC408, 0x44);
        rec = g_pad_ctx + D_80122820 * 0x250;
        FORMAT_SIGNED(buf2, *(u16 *)(rec + 0x636));
        STR_CAT(buf, buf2);
        STR_CAT_ENTRY(buf, D_800EC40A, 0x46);
        handle = func_800AF950(handle, ctx, buf, 4, D_80122990 + 0x8C, 0x84, 0, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }

    arg0->unk40B8 = handle;
    return 0;
}
