#include "common.h"

void func_8006D1EC(void);
void field_reset_global_color_scale(void);

extern s32 D_800F2278[];
extern s32 D_800F227C[];
extern s32 D_800F2280[];

/**
 * @brief Clear the three field draw-state globals and reset the colour scale.
 * @see decomp.me (100%) TODO
 */
void func_80067AA4(void)
{
    D_800F2280[0] = 0;
    D_800F227C[0] = 0;
    D_800F2278[0] = 0;
    func_8006D1EC();
    field_reset_global_color_scale();
}

/** @brief Packet tag, addressed whole or by its length byte. */
typedef union
{
    u32 word;
    struct
    {
        u8 _addr[3];        // 0x00
        u8 len;             // 0x03
    } f;
} PrimTag;

/** @brief Packet colour word, addressed whole or by its code byte. */
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

typedef struct
{
    u32 otag[0x1010];               // 0x0000
    u8 _pad[0x40B8 - 0x4040];       // 0x4040
    PrimTile* cursor;               // 0x40B8
} RenderHalf;

/**
 * @see decomp.me (100%) TODO
 */
void func_80067AE0(RenderHalf* ctx, s32 arg1)
{
    PrimTile* prim = ctx->cursor;

    prim->rgbc.word = 0x808080;
    if (arg1 >= 0x200)
    {
        prim->rgbc.word = 0x8080;
    }
    if (arg1 >= 0x300)
    {
        prim->rgbc.word = prim->rgbc.word >> 8;
    }
    prim->tag.f.len = 3;
    prim->rgbc.f.code = 0x40;
    prim->h = 0x10;
    prim->y0 = 0x10;
    prim->x0 = 0;
    prim->w = arg1 >> 2;
    prim->tag.word = (prim->tag.word & 0xFF000000) | (ctx->otag[0] & 0xFFFFFF);
    ctx->otag[0] = (ctx->otag[0] & 0xFF000000) | ((u32)prim & 0xFFFFFF);
    prim += 1;
    ctx->cursor = prim;
}
