#include "common.h"

/**
 * @brief 2D short vector used as an (x, y) position argument to the glyph
 *        drawing helpers.
 */
typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

/**
 * @brief Flat rectangle GPU primitive (code 0x62) used for the selection bar.
 */
typedef struct
{
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

extern s32 D_80165F80;
extern s32 D_80165FEC;
extern s32 D_80165FF4;
extern s32 D_801660A0;
extern s32 D_80166104;
extern s32 D_801663A8[];
extern s32 D_80166438;
extern u8 D_80166440[];
extern s32 D_80166A80[];
extern s32 D_80166AE0;

extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern char D_800ECFD0[];

extern u16 D_8014B038;
extern u16 D_8014B03A;
extern u16 D_8014B03C;
extern u16 D_8014B03E;
extern u16 D_8014B040;
extern u16 D_8014B048;
extern u16 D_8014B04A;
extern u16 D_8014B04C;
extern u16 D_8014B066;
extern u16 D_8014B06C;
extern u16 D_8014B072;
extern u16 D_8014B07A;
extern u16 D_8014B09E;
extern u16 D_8014B0E8;
extern u16 D_8014B0EC;

s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
s32 func_8001714C();
u8 *func_80143334(void *arg0);

/* Resolve a glyph pointer from a symbol whose low half stores a 16-bit offset. */
#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
/* Resolve a glyph pointer from a base pointer plus a 16-bit offset field. */
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/**
 * @brief Build the primitive list for the memory-card entry browser body.
 *
 * Dispatches on the current status code @c D_80165FEC to draw a status/prompt
 * glyph, or, in the default case, renders one row per card entry (rank digits,
 * icons, protect/copy state) plus the highlight bar for the selected row.
 *
 * @param ot Ordering table the primitives are linked into.
 * @param prim GPU packet cursor to emit primitives into.
 * @param x_offset Horizontal scroll offset subtracted from each glyph x.
 * @param y_offset Vertical scroll offset subtracted from each row y.
 * @return The advanced packet cursor past the last emitted primitive.
 * @note WIP - not yet byte-matching. Currently 99.69% (gcc272_cdk); residual is
 *       a five-row codegen difference near the entry loop (one instruction short).
 */
s32 func_80141250(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    if ((D_80165F80 & 7) != 0) {
        if (D_80165FEC >= 0xF8) {
            if (D_80165FEC < 0xFE) {
                return prim;
            }
            if (D_80165FEC == 0xFF) {
                return prim;
            }
        }
    }

    switch (D_80165FEC) {
    case 0xF8:
        do { prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B06C, 0x34), 4, -x_offset + 0x96, -y_offset, 2); } while (0);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0EC, 0xB4), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xF6:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A, 0x42), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFF: {
        s32 x = -x_offset + 0x96;
        u8 *base = (u8 *)&D_8014B038;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0), 4, x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
        break;
    }
    case 0xFA: {
        s32 x = -x_offset + 0x96;
        u8 *base = (u8 *)&D_8014B03A;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03A, 2), 4, x, -y_offset, 2);
        base -= 2;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x5A), 4, x, 0x10 - y_offset, 2);
        break;
    }
    case 0xF7: {
        s32 x = -x_offset + 0x96;
        u8 *base = (u8 *)&D_8014B03A;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03A, 2), 4, x, -y_offset, 2);
        base -= 2;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x60), 4, x, 0x10 - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x62), 4, x, 0x20 - y_offset, 2);
        break;
    }
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03C, 4), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B048, 0x10), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B04A, 0x12), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFE:
        break;
    default: {
        s32 row_y;
        s32 i;

        if (D_80166AE0 != 0) {
            s32 x = -x_offset + 0x96;
            u8 *base = (u8 *)&D_8014B038;
                prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0), 4, x, -y_offset, 2);
                prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
                prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            break;
        }

        do { i = 0; } while (0);
        if (D_80165FEC > 0) {
            s32 base_x;
            s32 *flag_ptr;
            u16 misc_glyph;
            Vec2s pos;
            s32 row;
            u8 *base;
            s32 misc_x;
            s32 entry_offset;
            s32 rank_offset;
            s32 rank_x;

            base = (u8 *)&D_8014B038;
            base_x = -x_offset;
            misc_x = base_x + 0xD6;
            entry_offset = 0;
            rank_offset = 0;
            do {
                row = ((i * 14) - y_offset) - D_80166104;
                row_y = row + 1;
                if ((u32)(row + 0xE) < 0x65U) {
                    flag_ptr = (s32 *)((u8 *)D_801663A8 + rank_offset);
                    if (*flag_ptr >= 0) {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A88A0(
                            func_800A8A78(ot, prim, *(s32 *)((u8 *)D_80166A80 + rank_offset), 4, &pos, 0),
                            ot, (void *)((s32)D_8014B066 + (s32)base), 4,
                            base_x + 0x70, row_y, 0);
                        pos.y = row_y;
                        pos.x = misc_x;
                        if ((D_80166438 - 1) == *flag_ptr) {
                            misc_glyph = *(u16 *)(base + 0x36);
                            rank_x = misc_x << 16;
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, rank_x >> 16, row_y, 0);
                        } else if (*flag_ptr < 2) {
                            misc_glyph = *(u16 *)(base + 0x38);
                            rank_x = misc_x << 16;
                            prim = func_800A88A0(prim, ot, (void *)((s32)misc_glyph + (s32)base), 4, rank_x >> 16, row_y, 0);
                        }
                        if (*func_80143334((void *)&((u8 (*)[0x320])D_80166440)[D_801660A0][entry_offset + 0xC]) == 0x2B) {
                            prim = func_800A88A0(prim, ot, (void *)((s32)D_8014B0E8 + (s32)base), 4, 0x10C - x_offset, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, (void *)&((u8 (*)[0x320])D_80166440)[D_801660A0][entry_offset], 0xC) == 0) {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_8014B03E + (s32)base), 4, 1 - x_offset, row_y, 0);
                    } else if (func_8001714C(D_800ECF8C, (void *)&((u8 (*)[0x320])D_80166440)[D_801660A0][entry_offset], 0xC) == 0) {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_8014B072 + (s32)base), 4, 1 - x_offset, row_y, 0);
                    } else if (func_8001714C(D_800ECFC4, (void *)&((u8 (*)[0x320])D_80166440)[D_801660A0][entry_offset], 8) == 0) {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_8014B04C + (s32)base), 4, 1 - x_offset, row_y, 0);
                    } else if (func_8001714C(D_800ECFD0, (void *)&((u8 (*)[0x320])D_80166440)[D_801660A0][entry_offset], 9) == 0) {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_8014B09E + (s32)base), 4, 1 - x_offset, row_y, 0);
                    } else {
                        prim = func_800A88A0(prim, ot, (void *)((s32)D_8014B040 + (s32)base), 4, 1 - x_offset, row_y, 0);
                    }
                }
                entry_offset += 0x28;
                rank_offset += 4;
                i++;
            } while (i < D_80165FEC);
        }

        row_y = ((D_80165FF4 * 14) - y_offset) - D_80166104;
        if (D_80166AE0 == 0) {
            TILE *tile = (TILE *)prim;
            *(u32 *)&tile->r0 = 0xF080F0;
            *((u8 *)tile + 3) = 3;
            tile->code = 0x62;
            tile->w = 0x12C;
            tile->x0 = 0;
            tile->y0 = row_y;
            tile->h = 0xE;
            tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
            *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
            prim += sizeof(TILE);
        }
        break;
    }
    }
    return prim;
}
