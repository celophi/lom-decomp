#include "common.h"
#include "sdk/libgpu.h"

/** @brief Screen translation applied to active effect primitives. */
typedef struct
{
    u16 x;
    u16 y;
} FieldEffectMotion;

extern POLY_FT4 D_80107800[];
extern FieldEffectMotion D_801077FC;
extern u16 D_801058E0[];
extern void bcopy(void *, void *, s32);

/**
 * @brief Fade, translate, and enqueue active primitives from the 256-entry effect pool.
 * @param buffer Ordering table and packet-buffer state, with the write cursor at offset 0x40B8.
 */
void func_80086FB8(u8 *buffer)
{
    POLY_FT4 *source;
    POLY_FT4 *output;
    u32 *ordering;
    s32 i;
    u8 color;

    output = *(POLY_FT4 **)(buffer + 0x40B8);
    ordering = (u32 *)buffer;
    source = D_80107800;

    for (i = 0; i < 256;)
    {
        color = source->r0;
        if (color != 0)
        {
            if (color < 0x10)
            {
                color = 0;
            }
            else
            {
                color -= 0x10;
            }

            source->b0 = color;
            source->g0 = color;
            source->r0 = color;
            source->code |= 2;
            source->x0 += D_801077FC.x;
            source->x1 += D_801077FC.x;
            source->x2 += D_801077FC.x;
            source->x3 += D_801077FC.x;
            source->y0 += D_801077FC.y;
            source->y1 += D_801077FC.y;
            source->y2 += D_801077FC.y;
            source->y3 += D_801077FC.y;

            bcopy(source, output, sizeof(POLY_FT4));
            addPrim(&ordering[D_801058E0[i]], output);
            output++;
        }

        i++;
        source++;
    }

    *(POLY_FT4 **)(buffer + 0x40B8) = output;
}
