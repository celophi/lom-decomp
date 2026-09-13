#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

/*
 * Encyclopedia (ZUKAN) overlay - all optimized (gcc272_cdk) functions.
 *
 * This single translation unit holds every ZUKAN function that compiles under
 * GCC 2.7.2 CDK, laid out in strict address order so the object reproduces the
 * original contiguous .text block. The overlay header word lives in
 * overlay_header.c (linked first, before the asm rodata blob) and func_80142D08
 * lives in its own file because it is the one function built at -O0.
 */

/* ---- Shared primitive/record types (identical layout across the overlay) ---- */

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ZukanRect;

typedef struct
{
    s16 x;
    s16 y;
    s16 clut_x;
    s16 clut_y;
} ZukanImageVramLayout;

/**
 * @brief One encyclopedia list entry: glyph index and a discovered flag in the
 *        top bit of @c field_2.
 */
typedef struct
{
    u16 field_0;
    u16 field_2;
} ZukanResourceEntry;

/**
 * @brief One per-entry glyph record. @c word0/word4 pack the sprite fields;
 *        @c field_8/field_A are the on-screen X/Y used when building primitives.
 */
typedef struct
{
    u32 word0;
    u32 word4;
    u16 field_8;
    u16 field_A;
} ZukanEntryRecord;

/**
 * @brief Flat triangle GPU primitive (code 0x22) used for the scroll arrows.
 */
typedef struct
{
    u_long tag;
    u_char r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
} ZukanPolyF3;

/**
 * @brief Persistent draw state for the encyclopedia screen.
 * @note Only the two ordering tables, the primitive cursor and the frame flag
 *       are referenced here; the padding preserves the original field offsets.
 */
typedef struct
{
    u8 pad0[0x30];
    s32 ot30;
    s32 ot34;
    u8 pad38[0x4008];
    void *prim_cursor;
    s16 unk4044;
    s16 frame_flag;
} ZukanDrawState;

/**
 * @brief 2D short position passed to the number/text drawing helpers.
 */
typedef struct
{
    s16 x;
    s16 y;
} ZukanPos;

typedef struct
{
    s32 tag;
    s32 color_and_code;
    s16 x0;
    s16 y0;
    s16 x1;
    u16 y1;
} ZukanLinePacket;

typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} ZukanFadePrimitive;

typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 steps_remaining;
} ZukanFadeState;

typedef struct
{
    u8 pad[0xC];
    s32 size;
    s32 data_offset;
} ZukanResourceHeader;

/* ---- outline/fade helpers (func_80142374 / func_801424EC) ---- */

#define ZUKAN_GPU_ADDRESS_MASK 0xFFFFFF
#define ZUKAN_GPU_TAG_HIGH_MASK 0xFF000000
#define ZUKAN_FADE_NEUTRAL 0x100
#define ZUKAN_FADE_ADDITIVE_THRESHOLD (ZUKAN_FADE_NEUTRAL + 1)
#define ZUKAN_FADE_ADDITIVE_DRAW_MODE 0x25
#define ZUKAN_FADE_SUBTRACTIVE_DRAW_MODE 0x45
#define ZUKAN_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((ZukanFadePrimitive*)((u8*)(primitive) + sizeof(type)))

/* ---- Globals shared by more than one function (consistent type) ---- */

extern s32 D_800F22AC;
extern u8 D_800EC3E0;
extern s32 D_80122988;
extern s32 D_80157528;
extern ZukanEntryRecord D_80157420[];
extern ZukanResourceEntry D_80157530[];
extern ZukanFadeState D_80157D30;
extern s16 D_80157D36;
extern s32 D_80157D38;
extern s32 D_80157D3C;
extern s32 D_80157D40;
extern ZukanFadeState D_80157D48;
extern s32 D_80157D50;
extern s32 D_80157D54;
extern s32 D_80157D58;
extern s32 D_80157D5C;
extern s32 D_80157D60;
extern s32 D_80157D64;
extern s32 D_80157D68;
extern s32 D_80157D70;
extern s32 D_80157D74;
extern s32 D_80157D78;

/*
 * D_8014471C, D_80157520 and D_80157D6C are referenced with different C types by
 * different functions (byte array vs scalar, and int vs pointer). Each is
 * declared locally inside the functions that use it so every function keeps the
 * exact type it was matched with.
 */

/* Forward declaration - func_80141144 calls func_801411C4 with a real prototype. */
s32 func_801411C4(ZukanImageVramLayout *destinations, u8 *tim);

/* ============================ 0x80141094 ============================ */

s32 func_80141094(s32 arg0, s32 arg1)
{
    extern s32 D_80157D6C;
    s32 ret;
    volatile s32 pad[2];

    D_80157D38 = arg1;
    D_80157D6C = (arg0 + 3) & ~3;
    func_80141144((ret = arg0 + 0x8000, arg0));
    func_800AA02C();
    func_801424D0(0x100, 0x100, 0x100, 6);
    D_80157D64 = 0;
    D_80157D58 = 1;
    D_80157D60 = 0;
    D_80157D74 = 0;
    D_80157D70 = 0;
    D_80157D68 = 0;
    func_801427E0(arg1);
    do {
        do {
            return ret;
        } while (0);
    } while (0);
}

/* ============================ 0x80141144 ============================ */

/**
 * @brief Upload the two UI image blocks used by the encyclopedia screen.
 * @see decomp.me (100%)
 */
void func_80141144(void)
{
    extern u8 D_8014471C[];
    ZukanImageVramLayout destinations;
    u8 *base = D_8014471C;

    destinations.x = 0x340;
    destinations.y = 0x100;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    func_801411C4(&destinations, base + *(s32 *)(base + 8));

    destinations.x = 0x140;
    destinations.y = 0;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1F2;
    func_801411C4(&destinations, base + *(s32 *)(base + 4));
}

/* ============================ 0x801411C4 ============================ */

/**
 * @brief Upload a TIM image and its optional CLUT to VRAM.
 * @return The TIM pixel-mode bits from the flags word.
 * @see decomp.me (100%)
 */
s32 func_801411C4(ZukanImageVramLayout *destinations, u8 *tim)
{
    ZukanRect upload_rect;
    s32 flags;
    s32 clut_block_size;
    u16 *pixel_dimensions;
    s32 mode;

    flags = *(s32 *)(tim + 4);
    clut_block_size = *(s32 *)(tim + 8);
    mode = flags & 7;

    if (flags & 8)
    {
        upload_rect.x = destinations->clut_x;
        upload_rect.y = destinations->clut_y;
        upload_rect.w = 0x100;
        upload_rect.h = 1;
        func_80019A34(&upload_rect, tim + 0x14);
        pixel_dimensions = (u16 *)(clut_block_size - (-(s32)tim) + 0x10);
    }
    else
    {
        pixel_dimensions = (u16 *)(tim + 0x10);
    }

    upload_rect.x = destinations->x;
    upload_rect.y = destinations->y;
    upload_rect.w = pixel_dimensions[0];
    upload_rect.h = pixel_dimensions[1];
    func_80019A34(&upload_rect, clut_block_size - (-(s32)tim) + 0x14);
    return mode;
}

/* ============================ 0x8014128C ============================ */

/**
 * @brief Build the current frame and advance the encyclopedia transition state.
 * @see decomp.me (100%)
 */
void func_8014128C(s32 arg0)
{
    func_801418A4(arg0);
    func_80141C08(arg0);
    func_80141DF4(arg0);
    D_800F22AC += 1;
    func_80141354();

    if (D_80157D68 != 0)
    {
        D_80157D70 += (D_80157D74 - D_80157D70) / D_80157D68;
        D_80157D68 -= 1;
        return;
    }
    D_80157D70 = D_80157D74;
}

/* ============================ 0x80141354 ============================ */

s32 func_80141354()
{
    s32 count;
    s32 moved;
    s32 limit;
    s32 max;
    s32 forward;
    s32 backward;
    s32 value;

    if (D_80157D64 != 0)
        return;

    if (D_80122988 & 0x800) {
        D_80157528 = 1;
        return;
    }

    if (D_80157D58 != 0) {
        if (D_80157D36 != 0)
            return;

        if (D_80122988 & 0x40) {
            D_80157528 = 1;
            return;
        }

        moved = 0;
        if (D_80122988 & 0x220) {
            if (D_80157530[D_80157D60].field_2 >> 15) {
                func_800A3938(0x7E, 0x80);
                func_801424D0(0, 0, 0, 6);
                func_80142884(D_80157D60);
                D_80157D64 = 6;
                D_80157D54 = 0;
            } else {
                func_800A3938(0x78, 0x80);
            }
            return;
        }

        count = 1;
        if (D_80122988 & 8) {
            count = 8;
            D_80122988 = 0x4000;
        } else if (D_80122988 & 4) {
            count = 8;
            D_80122988 = 0x1000;
        }

        if (count != 0) {
            forward = D_80122988 & 0x6000;
            backward = D_80122988 & 0x9000;
            max = D_80157D3C;
            limit = max - 1;
            do {
                if (forward) {
                    D_80157D60++;
                    if (D_80157D60 >= max)
                        D_80157D60 = 0;
                    moved = 1;
                } else if (backward) {
                    D_80157D60--;
                    if (D_80157D60 < 0)
                        D_80157D60 = limit;
                    moved = 1;
                }
                if ((D_80157D60 == limit) || (D_80157D60 == 0))
                    count = 1;
                count--;
            } while (count != 0);
        }

        if (moved != 0) {
            func_800A3938(0x7D, 0x80);
            func_80141740();
        }
        return;
    }

    if (D_80122988 & 0x260) {
        func_800A3938(0x7E, 0x80);
        func_801424D0(0, 0, 0, 6);
        D_80157D64 = 5;
        D_80157D54 = 0;
        return;
    }

    if (D_80122988 & 0x2008) {
        func_800A3938(0x7D, 0x80);
        func_80141B90();
        value = D_80157D78;
        if ((value != 0) && (D_80122988 & 0x2000)) {
            func_801428C4(value);
            return;
        }

        D_80157D60++;
        if (D_80157D60 >= D_80157D3C)
            D_80157D60 = 0;
        while ((D_80157530[D_80157D60].field_2 >> 15) == 0) {
            D_80157D60++;
            if (D_80157D60 >= D_80157D3C)
                D_80157D60 = 0;
        }
        func_80142884(D_80157D60);
        func_80141740();
        return;
    }

    if (D_80122988 & 0x8004) {
        func_800A3938(0x7D, 0x80);
        func_80141BCC();
        value = D_80157D40;
        if ((value != 0) && (D_80122988 & 0x8000)) {
            func_801428C4(value);
            return;
        }

        D_80157D60--;
        if (D_80157D60 < 0)
            D_80157D60 = D_80157D3C - 1;
        while ((D_80157530[D_80157D60].field_2 >> 15) == 0) {
            D_80157D60--;
            if (D_80157D60 < 0)
                D_80157D60 = D_80157D3C - 1;
        }
        func_80142884(D_80157D60);
        func_80141740();
    }
}

/* ============================ 0x80141740 ============================ */

void func_80141740(void)
{
    s32 value = D_80157D60 << 4;
    s32 delta = value - D_80157D70;

    if (delta < 0)
    {
        D_80157D74 = value;
        D_80157D68 = 4;
    }
    else if (delta > 0x70)
    {
        D_80157D74 = value - 0x70;
        D_80157D68 = 4;
    }
}

/* ============================ 0x80141794 ============================ */

/** @brief Append a fixed draw-mode packet to the ordering table. */
s32 func_80141794(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 5);
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/* ============================ 0x801417E8 ============================ */

/** @brief Append the alternate fixed draw-mode packet to the ordering table. */
s32 func_801417E8(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 0x1D);
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/* ============================ 0x8014183C ============================ */

/** @brief Append the draw-mode packet selected by the current page bits. */
s32 func_8014183C(s32 packet_cursor, s32 ordering_table)
{
    DR_TPAGE* draw_mode = (DR_TPAGE*)packet_cursor;

    setDrawTPage(draw_mode, 0, 0, 0x1E | ((D_80157D5C & 3) << 7));
    addPrim(ordering_table, draw_mode);
    return packet_cursor + sizeof(DR_TPAGE);
}

/* ============================ 0x801418A4 ============================ */

void func_801418A4(u8 *arg0)
{
    u8 *dst;
    s32 i;
    s32 cur;
    s32 one;
    volatile s32 pad[2];

    dst = arg0 + 0x28;
    i = 0;
    cur = *(s32 *)(arg0 + 0x4040);
    do {
        cur = func_80141988(cur, dst, i, D_80157420[i].field_8,
                           D_80157420[i].field_A, 0);
        i++;
    } while (i < 6);

    dst = arg0 + 0x3C;
    i = 6;
    one = 1;
    do {
        cur = func_80141988(cur, dst, i, D_80157420[i].field_8,
                           D_80157420[i].field_A, one);
        i++;
    } while (i < 0x15);

    *(s32 *)(arg0 + 0x4040) = func_801424EC(cur, arg0 + 0x2C);
}

/* ============================ 0x80141988 ============================ */

s32 func_80141988(s32 arg0, s32 *arg1, u32 arg2, s32 arg3, s32 arg4)
{
    ZukanEntryRecord *entry;
    DR_TPAGE *draw;

    SET_BGR0_PACKED(((SPRT *)arg0), GPU_TINT_NEUTRAL);

    if (D_80157D58 != 0) {
        if (arg2 < 4) {
            SET_BGR0_PACKED(((SPRT *)arg0), 0x303030);
        }
    } else if ((arg2 == 0) || (arg2 == 3) || (arg2 == 5)) {
        switch (D_80157D64) {
        case 1:
            if (arg2 == 3) {
                SET_BGR0_PACKED(((SPRT *)arg0), 0xE0E0FF);
            }
            break;
        case 3:
            if (arg2 == 0) {
                SET_BGR0_PACKED(((SPRT *)arg0), 0xE0E0FF);
            }
            break;
        case 5:
            if (arg2 == 5) {
                SET_BGR0_PACKED(((SPRT *)arg0), 0xE0E0FF);
            }
            break;
        }
    }

    setlen(((SPRT *)arg0), 4);
    ((SPRT *)arg0)->code = 0x64;
    ((SPRT *)arg0)->x0 = arg3 + 8;
    ((SPRT *)arg0)->y0 = arg4;

    entry = &D_80157420[arg2];
    ((SPRT *)arg0)->w = (entry->word4 >> 14) & 0x1FF;
    ((SPRT *)arg0)->h = entry->word4 >> 23;
    ((SPRT *)arg0)->u0 = entry->word0 >> 8;
    ((SPRT *)arg0)->v0 = entry->word4;
    ((SPRT *)arg0)->clut = ((entry->word4 >> 8) & 0x3F) | 0x7C80;

    addPrim(arg1, ((SPRT *)arg0));

    arg0 += sizeof(SPRT);
    draw = (DR_TPAGE *)arg0;
    switch ((u8)entry->word0) {
    case 0:
        setDrawTPage(draw, 0, 0, 5);
        addPrim(arg1, draw);
        arg0 += sizeof(DR_TPAGE);
        break;
    case 1:
        setDrawTPage(draw, 0, 0, 0x1D);
        addPrim(arg1, draw);
        arg0 += sizeof(DR_TPAGE);
        break;
    }

    return arg0;
}

/* ============================ 0x80141B90 ============================ */

void func_80141B90(void)
{
    func_801424D0(0, 0, 0, 6);
    D_80157D64 = 1;
    D_80157D54 = 0;
}

/* ============================ 0x80141BCC ============================ */

void func_80141BCC(void)
{
    func_801424D0(0, 0, 0, 6);
    D_80157D64 = 3;
    D_80157D54 = 0;
}

/* ============================ 0x80141C08 ============================ */

void func_80141C08(u8 *arg0)
{
    s32 saved;
    volatile s32 pad[2];
    if (D_80157D64 != 0) {
        saved = *(s32 *)(arg0 + 0x4040);
        switch (D_80157D64) {
        case 1: {
            s32 *counter = &D_80157D54;
            if (++*counter == 8) {
                func_80142B3C();
                D_80157D64 = 2;
                *counter = 0;
                D_80157D50 = D_80157D60;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        }
        case 2:
            if (++D_80157D54 == 8) D_80157D64 = 0;
            break;
        case 3: {
            s32 *counter = &D_80157D54;
            if (++*counter == 8) {
                func_80142B3C();
                D_80157D64 = 4;
                *counter = 0;
                D_80157D50 = D_80157D60;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        }
        case 4:
            if (++D_80157D54 == 8) {
                D_80157D64 = 0;
                D_80157D54 = 0;
            }
            break;
        case 5:
            if (++D_80157D54 == 8) {
                D_80157D64 = 2;
                D_80157D54 = 0;
                D_80157D58 = 1;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        case 6: {
            s32 *counter = &D_80157D54;
            if (++*counter == 8) {
                func_80142B3C();
                D_80157D64 = 2;
                D_80157D58 = 0;
                *counter = 0;
                D_80157D50 = D_80157D60;
                func_801424D0(0x100, 0x100, 0x100, 6);
            }
            break;
        }
        }
        *(s32 *)(arg0 + 0x4040) = saved;
    }
}

/* ============================ 0x80141DF4 ============================ */

/**
 * @brief Draw the encyclopedia screen: either the entry-list view (title, scroll
 *        arrows, per-entry glyphs and the selection highlight) or the detail
 *        view (backdrop, name, and the "current/total" counter).
 *
 * @param ctx Encyclopedia draw state; the primitive cursor is advanced in place.
 * @note WIP - not yet byte-matching. Currently 98.5% (gcc272_cdk); residual is a
 *       handful of argdiff rows near the per-entry glyph loop (two instructions
 *       short). An exact-size gcc272 variant matches at 97.02%.
 */
void func_80141DF4(ZukanDrawState *ctx)
{
    extern u8 D_8014471C;
    volatile s32 unused_pad[2];
    u8 draw_env[0x60];
    ZukanPos pos;
    u8 *prim;
    s32 *ot;
    s32 i;
    s32 row_y;
    u8 *base;
    u8 *fallback_sym;
    u8 *fallback_base;
    ZukanResourceEntry *entry;

    prim = ctx->prim_cursor;
    ot = &ctx->ot30;

    if (D_80157D58 != 0)
    {
        s32 table_off;
        u16 glyph_off;
        ZukanPolyF3 *tri;
        TILE *tile;
        u8 *env_prim;

        {
            u8 *title_base = &D_8014471C;
            table_off = *(s32 *)(title_base + 0x10);
            glyph_off = *(u16 *)(D_80157D38 * 2 + table_off + title_base);
            prim = (u8 *)func_800A88A0(prim, ot, title_base + glyph_off + table_off, 0xA, 0xA0, 0x22, 2);
        }

        if (D_80157D70 != 0)
        {
            tri = (ZukanPolyF3 *)prim;
            *(u32 *)&tri->r0 = 0xF08080;
            setlen(tri, 4);
            tri->code = 0x22;
            tri->x0 = 0x99;
            tri->x1 = 0xA0;
            tri->x2 = 0xA7;
            tri->y1 = 0x2E;
            tri->y0 = tri->y2 = 0x35;
            addPrim(ot, tri);
            prim += sizeof(ZukanPolyF3);
        }

        if (D_80157D70 + 0x80 < D_80157D3C * 0x10)
        {
            tri = (ZukanPolyF3 *)prim;
            *(u32 *)&tri->r0 = 0xF08080;
            setlen(tri, 4);
            tri->code = 0x22;
            tri->x0 = 0x99;
            tri->x1 = 0xA0;
            tri->x2 = 0xA7;
            tri->y1 = 0xBB;
            tri->y0 = tri->y2 = 0xB4;
            addPrim(ot, tri);
            prim += sizeof(ZukanPolyF3);
        }

        if (ctx->frame_flag != 8)
            func_8001C56C(draw_env, 0, 0xF0, 0x140, 0xE0);
        else
            func_8001C56C(draw_env, 0, 8, 0x140, 0xE0);
        func_8001A5D4(prim, draw_env);
        addPrim(ot, prim);
        prim += 0x40;

        i = 0;
        if (D_80157D3C > 0)
        {
            do { do { base = &D_8014471C; } while (0); } while (0);
            fallback_sym = &D_800EC3E0;
            fallback_base = fallback_sym - 0x1C;
            entry = D_80157530;
loop_head:
            row_y = (i * 0x10) - D_80157D70;
            if ((u32)(row_y + 0xF) >= 0x8F)
                goto loop_inc;

            pos.x = 0;
            pos.y = row_y;
            prim = (u8 *)func_800A8B04(ot, prim, i + 1, 0, &pos, 0);
            if (entry->field_2 >> 15)
                goto glyph_true;
            goto glyph_false;

glyph_true:
            table_off = *(s32 *)(base + 0xC);
            glyph_off = *(u16 *)(table_off + (entry->field_0 * 2 + base));
            prim = (u8 *)func_800A88A0(prim, ot, base + glyph_off + table_off, 0, 0x66, row_y, 2);
            goto loop_inc;

glyph_false:
            prim = (u8 *)func_800A88A0(prim, ot,
                fallback_base + fallback_sym[0] + (fallback_sym[1] << 8),
                0, 0x66, row_y, 2);

loop_inc:
            do { do {
                if (++i < D_80157D3C)
                {
                    entry++;
                    goto loop_head;
                }
            } while (0); } while (0);
        }

        row_y = (D_80157D60 * 0x10) - D_80157D70;
        if (row_y < 0)
            row_y = 0;
        else if (row_y >= 0x71)
            row_y = 0x70;

        tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0xF080F0;
        setlen(tile, 3);
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = row_y;
        tile->w = 0xB8;
        tile->h = 0xF;
        addPrim(ot, tile);
        prim += sizeof(TILE);

        env_prim = prim;
        if (ctx->frame_flag != 8)
            func_8001C56C(draw_env, 0x48, 0x126, 0xB8, 0x80);
        else
            func_8001C56C(draw_env, 0x48, 0x3E, 0xB8, 0x80);
        func_8001A5D4(env_prim, draw_env);
        addPrim(ot, env_prim);
        prim = env_prim + 0x40;
    }
    else
    {
        ot = &ctx->ot34;
        prim = (u8 *)func_801428F0(prim, ot);
        prim = (u8 *)func_801429C4(prim, ot);

        pos.x = 0x106;
        pos.y = 0xBD;
        prim = (u8 *)func_800AD524(prim, ot, 0xB, &pos, 1);

        pos.x = 0xEE;
        pos.y = 0xBD;
        prim = (u8 *)func_800AD208(ot, prim, D_80157D50 + 1, 3, &pos, 0);

        pos.x = 0x10E;
        pos.y = 0xBD;
        prim = (u8 *)func_800AD208(ot, prim, D_80157D3C, 3, &pos, 0);
    }

    ctx->prim_cursor = prim;
}

/* ============================ 0x80142374 ============================ */

/** @see GOLEM golem_emit_panel_outline (100%) */
ZukanLinePacket* func_80142374(ZukanLinePacket* packet, s32* ordering_table, s32 x, s32 y, s32 width, s32 height, s32 color)
{
    s32 temporary;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x;
    packet->y0 = y;
    packet->x1 = x + width;
    packet->y1 = y;
    temporary = ZUKAN_GPU_TAG_HIGH_MASK;
    packet->tag = (packet->tag & ZUKAN_GPU_TAG_HIGH_MASK) | (*ordering_table & ZUKAN_GPU_ADDRESS_MASK);
    *ordering_table = (*ordering_table & temporary) | ((s32)packet & ZUKAN_GPU_ADDRESS_MASK);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x + width;
    packet->y0 = y;
    packet->x1 = x + width;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x + width;
    temporary = y + height;
    packet->y0 = temporary;
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    packet++;

    packet->color_and_code = color;
    setlen(packet, 3);
    setcode(packet, 0x40);
    packet->x0 = x;
    packet->y0 = y;
    packet->x1 = x;
    packet->y1 = y + height;
    addPrim(ordering_table, packet);
    return packet + 1;
}

/* ============================ 0x801424D0 ============================ */

/** @see GOLEM golem_set_fade_target (100%) */
void func_801424D0(s16 red, s16 green, s16 blue, s16 steps)
{
    D_80157D30.red = red;
    D_80157D30.green = green;
    D_80157D30.blue = blue;
    D_80157D30.steps_remaining = steps;
}

/* ============================ 0x801424EC ============================ */

/** @see GOLEM golem_render_fade (100%) */
ZukanFadePrimitive* func_801424EC(ZukanFadePrimitive* primitive, u_long* ordering_table_tag)
{
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

    if (D_80157D30.steps_remaining != 0)
    {
        red_step = (D_80157D30.red - D_80157D48.red) / D_80157D30.steps_remaining;
        green_step = (D_80157D30.green - D_80157D48.green) / D_80157D30.steps_remaining;
        blue_step = (D_80157D30.blue - D_80157D48.blue) / D_80157D30.steps_remaining;
        D_80157D30.steps_remaining = D_80157D30.steps_remaining - 1;
        D_80157D48.red = D_80157D48.red + red_step;
        D_80157D48.green = D_80157D48.green + green_step;
        D_80157D48.blue = D_80157D48.blue + blue_step;
    }
    else
    {
        D_80157D48.red = D_80157D30.red;
        D_80157D48.green = D_80157D30.green;
        D_80157D48.blue = D_80157D30.blue;
    }

    if ((D_80157D48.red != ZUKAN_FADE_NEUTRAL) || (D_80157D48.green != D_80157D48.red) ||
        (D_80157D48.blue != D_80157D48.green))
    {
        if (D_80157D48.red >= ZUKAN_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = D_80157D48.red - 1;
            primitive->tile.g0 = D_80157D48.green - 1;
            primitive->tile.b0 = D_80157D48.blue - 1;
        }
        else
        {
            if (D_80157D48.red == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~D_80157D48.red;
            }
            if (D_80157D48.green == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~D_80157D48.green;
            }
            if (D_80157D48.blue == ZUKAN_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~D_80157D48.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        primitive->tile.w = SCREEN_WIDTH;
        draw_mode = ZUKAN_FADE_ADDITIVE_DRAW_MODE;
        SET_YX0(&primitive->tile, 0, 0);
        primitive->tile.h = SCREEN_HEIGHT;
        addPrim(ordering_table_tag, &primitive->tile);

        primitive = ZUKAN_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (D_80157D48.red < ZUKAN_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = ZUKAN_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = ZUKAN_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    return primitive;
}

/* ============================ 0x801427E0 ============================ */

void func_801427E0(s32 arg0)
{
    u32 first[0x200];
    u16 second[0x400];
    s32 count;
    s32 i;
    s32 limit;

    count = func_80142D08(arg0, first, second);
    D_80157D3C = count;
    i = 0;
    if (count > 0)
    {
        limit = count;
        do
        {
            if (first[i] != 0)
            {
                D_80157530[i].field_2 |= 0x8000;
                D_80157530[i].field_2 =
                    (D_80157530[i].field_2 & 0x8000) | ((u16)first[i] & 0x7FFF);
                D_80157530[i].field_0 = second[i * 2];
            }
            else
            {
                D_80157530[i].field_2 = 0;
                D_80157530[i].field_0 = 0;
            }
            i++;
        } while (i < limit);
    }
}

/* ============================ 0x80142884 ============================ */

void func_80142884(s32 index)
{
    extern s32 D_80157520;
    func_800141EC((D_80157530[index].field_2 & 0x7FFF) + 0xBFC, D_80157520);
}

/* ============================ 0x801428C4 ============================ */

void func_801428C4(s32 value)
{
    extern s32 D_80157520;
    s32 resource = D_80157520;

    func_800141EC((value + 0xBFC) & 0xFFFF, resource);
}

/* ============================ 0x801428F0 ============================ */

s32 func_801428F0(s32 arg0, s32 arg1)
{
    extern u8 *D_80157D6C;
    s32 i;
    s32 x;
    u16 *offsets;
    u16 *base_offsets;
    u8 *text;
    volatile u8 text_buffer[0x100];

    base_offsets = (u16 *)(D_80157D6C + *(s32 *)(D_80157D6C + 8));
    offsets = base_offsets;
    x = 0x1A;
    i = 0;
    do
    {
        s32 width;

        text = (u8 *)base_offsets + *offsets;
        width = 0x30;
        if (*text == 0x20)
        {
            u8 inner_space = 0x20;

            do
            {
                text++;
                width += 0xC;
            } while (*text == inner_space);
        }
        arg0 = func_800A88A0(arg0, arg1, text, 0, width, x, 0);
        offsets++;
        i++;
        x += 0xD;
    } while (i < 0xC);
    return arg0;
}

/* ============================ 0x801429C4 ============================ */

void *func_801429C4(SPRT *spr, s32 *ot)
{
    extern u8 *D_80157D6C;
    s32 temp;
    u8 *base = D_80157D6C;
    u8 *data = base + *(s32 *)(base + 4);
    s32 count = *(u16 *)data + (*(u16 *)(data + 2) << 8);

    data += 4;
    if (count != 0) {
        do {
            SET_BGR0_PACKED(spr, GPU_TINT_NEUTRAL);
            setlen(spr, 4);
            temp = 0x64;
            spr->code = temp;
            spr->x0 = *(u16 *)data; data += 2;
            spr->y0 = *(u16 *)data; data += 2;
            spr->u0 = *data; data += 2;
            spr->v0 = *data; data += 2;
            spr->w = *(u16 *)data; data += 2;
            spr->h = *(u16 *)data; data += 2;
            if (D_80157D5C != 0) {
                spr->clut = 0x7B80;
            } else {
                spr->clut = (*(u16 *)data & 0x3F) | 0x7B80;
            }
            data += 4;
            addPrim(ot, spr);
            spr++;
            temp = count - 1;
            count = temp;
        } while (count != 0);
    }

    {
        DR_TPAGE *mode = (DR_TPAGE *)spr;
        setDrawTPage(mode, 0, 0, getTPage(D_80157D5C, 0, 0x380, 0x100));
        addPrim(ot, mode);
        return mode + 1;
    }
}

/* ============================ 0x80142B3C ============================ */

/**
 * @brief Copy the current encyclopedia resource, upload its TIM image, and cache its metadata.
 */
void func_80142B3C(void)
{
    extern u8 *D_80157520;
    extern u8 *D_80157D6C;
    ZukanImageVramLayout destinations;
    ZukanRect upload_rect;
    ZukanImageVramLayout *layout;
    ZukanRect *rect;
    ZukanResourceHeader *header;
    u8 *src;
    u8 *dst;
    u8 *end;
    u8 *tim;
    s32 flags;
    s32 clut_block_size;
    u16 *pixel_dimensions;
    s32 mode;

    func_80013F2C();

    header = (ZukanResourceHeader *)D_80157520;
    dst = D_80157D6C;
    end = (u8 *)header + header->size;
    src = (u8 *)header;
    if (src != end)
    {
        do
        {
            *dst++ = *src++;
        } while (src != end);
    }

    destinations.x = 0x380;
    destinations.y = 0x100;
    destinations.clut_x = 0;
    destinations.clut_y = 0x1EE;

    rect = &upload_rect;
    layout = &destinations;
    tim = D_80157520 + ((ZukanResourceHeader *)D_80157520)->size;
    flags = *(s32 *)(tim + 4);
    clut_block_size = *(s32 *)(tim + 8);
    mode = flags & 7;

    if (flags & 8)
    {
        upload_rect.x = layout->clut_x;
        upload_rect.y = layout->clut_y;
        rect->w = 0x100;
        rect->h = 1;
        func_80019A34(rect, tim + 0x14);
        pixel_dimensions = (u16 *)(clut_block_size + (s32)tim + 0x10);
    }
    else
    {
        pixel_dimensions = (u16 *)(tim + 0x10);
    }

    upload_rect.x = layout->x;
    upload_rect.y = layout->y;
    upload_rect.w = pixel_dimensions[0];
    upload_rect.h = pixel_dimensions[1];
    func_80019A34(&upload_rect, clut_block_size + (s32)tim + 0x14);

    {
        ZukanResourceHeader *resource = (ZukanResourceHeader *)D_80157520;
        D_80157D5C = mode;
        D_80157D40 = *(u16 *)((u8 *)resource + resource->data_offset);
        D_80157D78 = *(u16 *)((u8 *)resource + resource->data_offset + 2);
    }
}

/* ============================ 0x80142CA0 ============================ */

void func_80142CA0(void)
{
    extern u8 D_8014471C[];
    ZukanRect rect;
    s32 raw;
    s32 dimensions;
    s16 temp;
    u8 *base = D_8014471C;

    func_800141EC(0x5E3, base);

    raw = *(volatile s32 *)base;
    temp = raw;
    dimensions = temp;

    *(volatile s16 *)&rect.x = 0x340;
    rect.y = 0x100;
    rect.w = dimensions;

    dimensions = raw >> 16;
    rect.h = dimensions;

    func_80019A34(&rect, base + 4);
}
