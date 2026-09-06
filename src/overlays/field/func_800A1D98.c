#include "common.h"

typedef struct
{
    s32 unk0;
    u8 pad4[4];
    s32 unk8;
} ArgRec;

extern s32 D_801178E0;
extern u8 D_801178E8[];
extern s32 D_80117E68;
extern s32 D_80117E6C;
extern s32 D_80117E70;
extern s32 D_80117E74;
extern s32 D_80117E78;
extern s32 D_80117E7C;
extern s32 D_80117E80;
extern s32 D_80117E84;

s32 rand(void);
s32 rcos(s32 angle);
s32 rsin(s32 angle);

/**
 * @brief Initialize the field effect entries associated with an actor record.
 * @param arg0 Actor record that supplies the base coordinates.
 * @param arg1 Magnitude used to offset each generated entry.
 * @param arg2 Nonzero to randomize the generated magnitude.
 * @param arg3 Effect-entry group index.
 */
void func_800A1D98(ArgRec *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 i;
    s32 angle;
    s32 mag;
    u8 *base;
    u8 *entry;

    i = 0;
    base = D_801178E8;
    entry = base + arg3 * 0x2C;
    D_80117E80 = 3;
    D_801178E0 = 0x14;
    D_80117E84 = 0xA;
    do
    {
        angle = rand() >> 3;
        if (arg2 != 0)
        {
            mag = (s32)((rand() | 0x4000) * arg1) >> 0xF;
        }
        else
        {
            mag = arg1;
        }
        *(s16 *)(entry + 0) = (s16)(((s32)(rcos(angle) * mag) >> 0xC) + ((s32)arg0->unk0 >> 8));
        i += 1;
        *(s16 *)(entry + 0x16) = (s16)(((s32)(rsin(angle) * mag) >> 0xC) + ((s32)arg0->unk8 >> 8));
        entry += 2;
    } while (i < 0xA);
    D_80117E70 = D_80117E84;
    D_80117E78 = D_80117E84 + D_80117E80;
    D_80117E68 = D_80117E80 + 1;
    D_80117E74 = D_80117E68 >> 1;
    D_80117E7C = D_80117E84 + (D_80117E80 * 2);
    D_80117E6C = (D_801178E0 * D_80117E84) + 1;
}
