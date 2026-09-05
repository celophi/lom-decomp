#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

typedef struct {
    u32 word0;
    u32 word4;
    u16 field_8;
    u16 field_A;
} ZukanEntryRecord;

extern ZukanEntryRecord D_80157420[];
extern s32 D_80157D58;
extern s32 D_80157D64;

s32 func_80141988(s32 arg0, s32 *arg1, u32 arg2, s32 arg3, s32 arg4)
{
    ZukanEntryRecord *entry;
    DR_TPAGE *draw;

    SET_BGR0_PACKED(((SPRT *)arg0), GPU_TINT_NEUTRAL);

    if (D_80157D58 != 0) {
        if (arg2 < 4) {
            SET_BGR0_PACKED(((SPRT *)arg0), 0x303030);
        }
    } else if ((arg2 == 0) || (arg2 == 3) || (arg2 == 5)) {
        switch (D_80157D64) {
        case 1:
            if (arg2 == 3) {
                SET_BGR0_PACKED(((SPRT *)arg0), 0xE0E0FF);
            }
            break;
        case 3:
            if (arg2 == 0) {
                SET_BGR0_PACKED(((SPRT *)arg0), 0xE0E0FF);
            }
            break;
        case 5:
            if (arg2 == 5) {
                SET_BGR0_PACKED(((SPRT *)arg0), 0xE0E0FF);
            }
            break;
        }
    }

    setlen(((SPRT *)arg0), 4);
    ((SPRT *)arg0)->code = 0x64;
    ((SPRT *)arg0)->x0 = arg3 + 8;
    ((SPRT *)arg0)->y0 = arg4;

    entry = &D_80157420[arg2];
    ((SPRT *)arg0)->w = (entry->word4 >> 14) & 0x1FF;
    ((SPRT *)arg0)->h = entry->word4 >> 23;
    ((SPRT *)arg0)->u0 = entry->word0 >> 8;
    ((SPRT *)arg0)->v0 = entry->word4;
    ((SPRT *)arg0)->clut = ((entry->word4 >> 8) & 0x3F) | 0x7C80;

    addPrim(arg1, ((SPRT *)arg0));

    arg0 += sizeof(SPRT);
    draw = (DR_TPAGE *)arg0;
    switch ((u8)entry->word0) {
    case 0:
        setDrawTPage(draw, 0, 0, 5);
        addPrim(arg1, draw);
        arg0 += sizeof(DR_TPAGE);
        break;
    case 1:
        setDrawTPage(draw, 0, 0, 0x1D);
        addPrim(arg1, draw);
        arg0 += sizeof(DR_TPAGE);
        break;
    }

    return arg0;
}
