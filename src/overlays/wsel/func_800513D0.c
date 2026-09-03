#include "common.h"
#include "sdk/libgpu.h"

void *func_800513D0(TILE *tile, u_long *ot, s16 x, s16 y, s32 intensity)
{
    DR_TPAGE *draw_mode;

    setTile(tile);
    if (intensity < 0x100)
    {
        tile->b0 = -intensity;
        tile->g0 = -intensity;
        tile->r0 = -intensity;
    }
    else
    {
        tile->b0 = intensity;
        tile->g0 = intensity;
        tile->r0 = intensity;
    }
    setXY0(tile, x, y);
    setWH(tile, 0x10, 0x10);
    setSemiTrans(tile, 1);
    addPrim(ot, tile);

    draw_mode = (DR_TPAGE *)(tile + 1);
    if (intensity < 0x100)
    {
        setDrawTPage(draw_mode, 0, 0, 0x40);
    }
    else
    {
        setDrawTPage(draw_mode, 0, 0, 0x20);
    }
    addPrim(ot, draw_mode);
    draw_mode++;
    return draw_mode;
}
