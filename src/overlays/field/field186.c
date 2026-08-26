#include "common.h"

typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
} Struct_D80051ECC;

extern Struct_D80051ECC D_80051ECC;

void func_800AAFEC(Struct_D80051ECC *arg0);

void func_800C72A4(void)
{
    Struct_D80051ECC sp10;

    sp10 = D_80051ECC;
    func_800AAFEC(&sp10);
}

typedef struct
{
    u8 unk0;
    u8 pad1[0xD];
    s16 unkE;
    u8 pad10[4];
    u16 unk14;
} UnkStruct80122C02;

extern UnkStruct80122C02 D_80122C02;
extern u16 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 D_80045ECC[];
extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

/**
 * @brief Record the current gosub result index and count, then emit its portrait icon.
 */
void func_800C72E4(void)
{
    s32 temp;

    D_80122C02.unkE = temp = g_gosub_result_values[0];
    D_80122C02.unk14 = g_gosub_result_count;
    func_800B2844(D_80122C02.unk0, (temp * 0x60) + D_80045ECC, 0xFF);
}
