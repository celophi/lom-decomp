#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

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
 * @brief One encyclopedia list entry: glyph index and a discovered flag in the
 *        top bit of @c field_2.
 */
typedef struct
{
    u16 field_0;
    u16 field_2;
} ZukanResourceEntry;

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

extern u8 D_8014471C;
extern u8 D_800EC3E0;
extern ZukanResourceEntry D_80157530[];
extern s32 D_80157D38;
extern s32 D_80157D3C;
extern s32 D_80157D50;
extern s32 D_80157D58;
extern s32 D_80157D60;
extern s32 D_80157D70;

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
