#include "common.h"

typedef union
{
    u32 word;
    struct
    {
        u8 _addr[3];        // 0x00
        u8 len;             // 0x03
    } f;
} PrimTag;

typedef union
{
    u32 word;
    struct
    {
        u8 r;               // 0x04
        u8 g;               // 0x05
        u8 b;               // 0x06
        u8 code;            // 0x07
    } f;
} PrimRgbc;

/** @brief 16-byte flat tile primitive. */
typedef struct
{
    PrimTag tag;            // 0x00
    PrimRgbc rgbc;          // 0x04
    s16 x0;                 // 0x08
    s16 y0;                 // 0x0A
    s16 w;                  // 0x0C
    s16 h;                  // 0x0E
} PrimTile;

/** @brief 8-byte draw-mode primitive. */
typedef struct
{
    PrimTag tag;            // 0x00
    u32 code;               // 0x04
} PrimMode;

/** @brief Fade colour triple plus its remaining step count. */
typedef struct
{
    s16 r;                  // 0x00
    s16 g;                  // 0x02
    s16 b;                  // 0x04
    s16 steps;              // 0x06
} FieldFade;

typedef struct
{
    u32 otag[0x1010];               // 0x0000
    u8 _pad[0x40B8 - 0x4040];       // 0x4040
    PrimTile* cursor;               // 0x40B8
} RenderHalf;

extern FieldFade g_field_fade_target;
extern FieldFade g_field_fade_current;

/**
 * @brief Advance the screen fade one step and emit its blend tile + draw mode.
 * @param ctx Render half whose OT slot 0x40 the packets are linked into.
 * @see decomp.me (100%) TODO
 */
void func_80067BBC(RenderHalf* ctx)
{
    PrimTile* prim = ctx->cursor;
    u32* ot = &ctx->otag[0x10];
    PrimMode* mode_prim;
    s32 dr;
    s32 dg;
    s32 db;
    s32 mode;

    if (g_field_fade_target.steps != 0)
    {
        dr = (g_field_fade_target.r - g_field_fade_current.r) / g_field_fade_target.steps;
        dg = (g_field_fade_target.g - g_field_fade_current.g) / g_field_fade_target.steps;
        db = (g_field_fade_target.b - g_field_fade_current.b) / g_field_fade_target.steps;
        g_field_fade_target.steps = g_field_fade_target.steps - 1;
        g_field_fade_current.r = g_field_fade_current.r + dr;
        g_field_fade_current.g = g_field_fade_current.g + dg;
        g_field_fade_current.b = g_field_fade_current.b + db;
    }
    else
    {
        g_field_fade_current.r = g_field_fade_target.r;
        g_field_fade_current.g = g_field_fade_target.g;
        g_field_fade_current.b = g_field_fade_target.b;
    }
    if ((g_field_fade_current.r != 0x100) ||
        (g_field_fade_current.g != g_field_fade_current.r) ||
        (g_field_fade_current.b != g_field_fade_current.g))
    {
        if (g_field_fade_current.r >= 0x101)
        {
            prim->rgbc.f.r = g_field_fade_current.r - 1;
            prim->rgbc.f.g = g_field_fade_current.g - 1;
            prim->rgbc.f.b = g_field_fade_current.b - 1;
        }
        else
        {
            if (g_field_fade_current.r == 0x100)
            {
                prim->rgbc.f.r = 0;
            }
            else
            {
                prim->rgbc.f.r = ~g_field_fade_current.r;
            }
            if (g_field_fade_current.g == 0x100)
            {
                prim->rgbc.f.g = 0;
            }
            else
            {
                prim->rgbc.f.g = ~g_field_fade_current.g;
            }
            if (g_field_fade_current.b == 0x100)
            {
                prim->rgbc.f.b = 0;
            }
            else
            {
                prim->rgbc.f.b = ~g_field_fade_current.b;
            }
        }
        prim->tag.f.len = 3;
        prim->rgbc.f.code = 0x62;
        prim->w = 0x140;
        mode = 0x25;
        prim->y0 = 0;
        prim->x0 = 0;
        prim->h = 0xF0;
        prim->tag.word = (prim->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((u32)prim & 0xFFFFFF);
        prim += 1;
        mode_prim = (PrimMode*)prim;
        if (g_field_fade_current.r < 0x101)
        {
            mode = 0x45;
        }
        mode_prim->tag.f.len = 1;
        mode_prim->code = mode | 0xE1000000;
        mode_prim->tag.word = (mode_prim->tag.word & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((u32)mode_prim & 0xFFFFFF);
        mode_prim += 1;
        prim = (PrimTile*)mode_prim;
    }
    ctx->cursor = prim;
}
