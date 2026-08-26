#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct
{
    u8 pad0[0x1C];
    u32 unk1C;
    u8 pad20[0x54 - 0x20];
} FieldObjRec86184;

extern FieldObjRec86184 D_800FDF58[];
extern u8 D_800FDCEA;
extern u16 D_800FE01E;

/**
 * @brief Build the textured sprite primitive for a field object icon.
 * @param sprt Sprite primitive to populate.
 * @param ot Ordering table the primitive (and its tpage) are linked into.
 * @param index Object slot index; selects clut/tpage and a special case at 2.
 * @param xy Packed screen position copied into the sprite's x0/y0.
 * @return Pointer just past the appended DR_TPAGE primitive.
 */
void *func_80086184(SPRT *sprt, u32 *ot, s32 index, u32 *xy)
{
    DR_TPAGE *mode;

    *(u32 *)&sprt->r0 = 0x808080;
    setSprt(sprt);
    *(u32 *)&sprt->x0 = *xy;
    *(u16 *)&sprt->u0 = 0xE800;
    *(u32 *)&sprt->w = 0x180018;

    if (index == 2 && D_800FDCEA >= 0x41)
    {
        sprt->clut = (((D_800FE01E & 3) + 0x1EF) << 6) | 0x10;
    }
    else
    {
        sprt->clut = ((index + 0x1F4) << 6) | ((D_800FDF58[index].unk1C >> 19) & 0xF);
    }

    addPrim(ot, sprt);

    mode = (DR_TPAGE *)(sprt + 1);
    if (index >= 2)
    {
        setDrawTPage(mode, 0, 0, getTPage(0, 1, 0x340 - (index << 6), 0));
    }
    else
    {
        setDrawTPage(mode, 0, 0, getTPage(0, 1, 0x380 - (index << 7), 0));
    }
    addPrim(ot, mode);
    return mode + 1;
}
