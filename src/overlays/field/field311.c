#include "common.h"

/**
 * @brief Placeholder layout for the record returned by func_800C1E40; only the
 *        byte at offset 4 is read here.
 */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
} Rec;

u8 *func_800C1E40(s32 arg0);
void func_800B2844(s32 arg0, void *arg1, s32 arg2);

extern s16 D_80122C0C;

/**
 * @see decomp.me (100%) TODO
 */
void func_800C7014(void)
{
    s32 idx;
    s32 k;
    u8 *p1;
    s32 value;

    idx = D_80122C0C;
    p1 = func_800C1E40(0x101);
    k = idx * 2;
    value = ((Rec *)(p1 + k))->unk4 +
            (((Rec *)(func_800C1E40(0x101) + (k += 1)))->unk4 << 8);
    func_800B2844(0, func_800C1E40(0x101) + (value + 4), 0xFF);
}
