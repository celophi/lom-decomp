#include "common.h"
#include "sdk/libgpu.h"
#include "gpu_packet.h"

extern s32 D_80157D5C;
extern u8 *D_80157D6C;

void *func_801429C4(SPRT *spr, s32 *ot)
{
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
