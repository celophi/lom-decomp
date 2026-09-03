#include "common.h"

typedef struct {
    u8 pad0[0x18];
    u32 unk18;
    u8 pad1C[0x26 - 0x1C];
    u16 unk26;
    u16 accum[4];
    u16 vals[8];
    u8 pad40[0x74 - 0x40];
    u16 unk74;
} Rec;

void func_800B7B98(Rec *arg0)
{
    s32 i;
    s32 j;
    u8 *entry;
    u16 v;

    arg0->unk26 = arg0->unk74;
    if ((arg0->unk18 & 0x7F) != 3) {
        for (i = 3; i >= 0; i--) {
            arg0->accum[i] = 0;
        }
        for (i = 1; i < 4; i++) {
            entry = (u8 *)arg0 + 0x90 + (i - 1) * 0x40;
            if (*entry != 0) {
                for (j = 0; j < 4; j++) {
                    arg0->accum[j] += *(u16 *)(entry + 0x24 + j * 2);
                }
            }
        }
    }
    for (i = 0; i < 8; i++) {
        v = arg0->vals[i] & 0x1FF;
        arg0->vals[i] = v | ((v >> 2) << 9);
    }
}
