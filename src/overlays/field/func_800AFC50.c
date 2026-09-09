#include "common.h"
#include "sdk/libgpu.h"

extern void bcopy(void *, void *, s32);
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
