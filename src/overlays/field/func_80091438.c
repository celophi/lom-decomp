#include "common.h"

/** @brief D_800FD818 object entry (stride 0x268). */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 pad2[0x268 - 0x2];
} Entry268;

/** @brief D_8010A038 animation record 8-byte sub-slot; unk6 is the value this function writes. */
typedef struct
{
    u8 pad0[0x6];
    u16 unk6;
} Slot8;

extern u8 D_800EB21C[];
extern u8 D_800EB224[];
extern u8 D_800EB22C[];
extern u8 D_800EB234[];
extern Entry268 D_800FD818[];
extern Slot8 D_8010A038[][50];

/**
 * @brief Set five animation slot values using the object's kind-specific table.
 * @param arg0 Object and animation record index.
 */
void func_80091438(s32 arg0)
{
    s32 i;

    for (i = 0; i < 5; i++)
    {
        switch (D_800FD818[arg0].unk1)
        {
        case 5:
        case 7:
        case 8:
        case 9:
            D_8010A038[arg0][D_800EB21C[i]].unk6 = D_800EB22C[i];
            break;
        case 10:
            D_8010A038[arg0][D_800EB21C[i]].unk6 = D_800EB234[i];
            break;
        default:
            D_8010A038[arg0][D_800EB21C[i]].unk6 = D_800EB224[i];
            break;
        }
    }
}
