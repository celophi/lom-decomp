#include "common.h"
#include "sdk/libgpu.h"

extern void bcopy(void *, void *, s32);

s32 field_text_build_sprites(SPRT *, u8 *, u16);
void field_text_reset_scratch(void);
void func_80063194(void);
void func_80086F48(POLY_FT4 *, s32);
POLY_FT4 *func_800AFC50(u32 *, POLY_FT4 *);

/**
 * @brief Render text through a scratch texture as a scaled, optionally outlined quad.
 * @param arg0 First free output primitive.
 * @param arg1 Ordering-table entry.
 * @param arg2 Text to render.
 * @param arg3 Text style.
 * @param arg4 Horizontal position.
 * @param arg5 Vertical position.
 * @param arg6 Alignment and outline flags.
 * @param arg7 Scratch texture row.
 * @param arg8 Horizontal scale in units of 1/256.
 * @param arg9 Vertical scale in units of 1/256.
 * @param arg10 Horizontal offset of the lower quad edge before scaling.
 * @param arg11 Enable primitive adjustment through func_80086F48.
 * @return First free primitive after the text and optional outline.
 */
POLY_FT4 *func_800AF950(POLY_FT4 *arg0, u32 *arg1, u8 *arg2, u16 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11)
{
    RECT rect;
    SPRT sprites[10];
    s16 temp_s0_2;
    s16 temp_v0;
    s16 temp_v1_2;
    s16 var_a3;
    s32 temp_s0;
    s32 temp_v1;
    s32 var_a0;
    s32 var_s3;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v1;
    s8 temp_v0_2;
    s8 temp_v0_3;
    s8 temp_v1_3;
    POLY_FT4 *var_s2;

    var_s3 = arg4;
    var_s2 = arg0;
    field_text_reset_scratch();
    temp_s0 = field_text_build_sprites(sprites, arg2, arg3);
    func_80063194();
    if (temp_s0 < 2)
    {
        temp_s0_2 = arg7 * 0x10;
        rect.x = 0x140;
        rect.w = 0x40;
        rect.y = temp_s0_2;
        rect.h = 0x10;
        ClearImage(&rect, 0, 0, 0);
        DrawSync(0);
        var_a3 = sprites[0].w;
        rect.x = (sprites[0].u0 >> 2) + 0x3C0;
        rect.y = sprites[0].v0 + 0x100;
        if (var_a3 < 0)
        {
            var_a3 += 3;
        }
        rect.w = (s16) (var_a3 >> 2);
        rect.h = sprites[0].h;
        MoveImage(&rect, 0x140, temp_s0_2);
        temp_v1 = arg6 & 0x7F;
        if (temp_v1 != 1)
        {
            if (temp_v1 == 2)
            {
                var_v0 = (s32) ((u16) sprites[0].w << 0x10) >> 0x11;
                goto block_7;
            }
        }
        else
        {
            var_v0 = (s32) sprites[0].w;
block_7:
            var_v0_2 = var_v0 * arg8;
            if (var_v0_2 < 0)
            {
                var_v0_2 += 0xFF;
            }
            var_s3 -= var_v0_2 >> 8;
        }
        *(u32 *)&var_s2->r0 = 0x808080;
        ((u8 *)var_s2)[3] = 9;
        var_s2->code = 0x2C;
        var_s2->x0 = (s16) var_s3;
        var_v1 = sprites[0].w * arg8;
        if (var_v1 < 0)
        {
            var_v1 += 0xFF;
        }
        var_a0 = arg10 * arg8;
        var_s2->x1 = (s16) (var_s3 + (var_v1 >> 8));
        if (var_a0 < 0)
        {
            var_a0 += 0xFF;
        }
        temp_v1_2 = var_s3 + (var_a0 >> 8);
        var_s2->x2 = temp_v1_2;
        var_v0_3 = sprites[0].w * arg8;
        if (var_v0_3 < 0)
        {
            var_v0_3 += 0xFF;
        }
        var_s2->x3 = (s16) (temp_v1_2 + (var_v0_3 >> 8));
        var_s2->y1 = (s16) arg5;
        var_s2->y0 = (s16) arg5;
        var_v0_4 = ((s16) sprites[0].h - 1) * arg9;
        if (var_v0_4 < 0)
        {
            var_v0_4 += 0xFF;
        }
        temp_v0 = arg5 + (var_v0_4 >> 8);
        temp_v1_3 = arg7 * 0x10;
        var_s2->y3 = temp_v0;
        var_s2->y2 = temp_v0;
        var_s2->u2 = 0;
        var_s2->u0 = 0;
        var_s2->v1 = temp_v1_3;
        var_s2->v0 = temp_v1_3;
        temp_v0_2 = (u8) sprites[0].w + 1;
        var_s2->u3 = temp_v0_2;
        var_s2->u1 = temp_v0_2;
        temp_v0_3 = ((u8) sprites[0].h + temp_v1_3) - 1;
        var_s2->v3 = temp_v0_3;
        var_s2->v2 = temp_v0_3;
        var_s2->tpage = 0x25;
        var_s2->clut = sprites[0].clut;
        var_s2->tag = (s32) ((var_s2->tag & 0xFF000000) | (*arg1 & 0xFFFFFF));
        *arg1 = (*arg1 & 0xFF000000) | ((s32) var_s2 & 0xFFFFFF);
        if (arg11 != 0)
        {
            func_80086F48(var_s2, 0);
        }
        var_s2++;
        if (arg6 & 0x80)
        {
            var_s2 = func_800AFC50(arg1, var_s2);
        }
    }
    return var_s2;
}

/**
 * @brief Add four black, one-pixel-offset copies of the preceding textured quad.
 * @param ot Ordering-table entry receiving the outline primitives.
 * @param output First free primitive slot, immediately after the quad to copy.
 * @return First free primitive slot after the four outline copies.
 * @note 100% match with GCC 2.7.2 CDK: 113 instructions, 452 bytes.
 */
POLY_FT4 *func_800AFC50(u32 *ot, POLY_FT4 *output)
{
    POLY_FT4 *source = output;
    POLY_FT4 *poly = output;
    s32 i = 0;
    do
    {
        bcopy(source - 1, poly, 0x28);
        poly->b0 = 0;
        poly->g0 = 0;
        poly->r0 = 0;
        switch (i)
        {
        case 0:
            poly->x0++;
            poly->x1++;
            poly->x2++;
            poly->x3++;
            break;
        case 1:
            poly->x0--;
            poly->x1--;
            poly->x2--;
            poly->x3--;
            break;
        case 2:
            poly->y0++;
            poly->y1++;
            poly->y2++;
            poly->y3++;
            break;
        default:
            poly->y0--;
            poly->y1--;
            poly->y2--;
            poly->y3--;
            break;
        }
        addPrim(ot, poly);
        poly++;
        i++;
    } while (i < 4);
    return poly;
}
