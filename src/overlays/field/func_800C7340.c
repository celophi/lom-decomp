#include "common.h"

void func_800B2844();
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

extern u8 D_800F0E98[];
extern u8 D_80045ECC[];
extern s16 D_80122C10;
extern u16 D_80122C16;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Dispatch the selected menu record's extra slots and fixed trailing slot.
 * @note Skip 0xFE/0xFF entries and record the count of dispatched extra slots.
 * @note Selections of five or greater issue an AKAO command instead.
 * @note WIP: instruction ordering and temporary-register differences remain.
 */
void func_800C7340(void)
{
    s32 temp_s2;
    s32 i;
    s32 count;
    s32 rec_off;
    u8 *p;
    u8 flag;
    s32 tbl_off;

    temp_s2 = D_80122C10;
    if (temp_s2 < 5)
    {
        count = 0;
        i = 0;
        rec_off = temp_s2 * 0x60;
        do
        {
            p = &g_menuLayoutBuffer[i + rec_off];
            flag = p[0x2F38];
            if (flag != 0xFF)
            {
                if (flag != 0xFE)
                {
                    tbl_off = flag * 2;
                    func_800B2844(count, D_800F0E98[tbl_off] + (D_800F0E98[tbl_off + 1] << 8) + D_800F0E98, 0xFF);
                    count++;
                }
            }
            i++;
        } while (i < 3);
        func_800B2844(3, (temp_s2 * 0x60) + D_80045ECC, 0xFF);
        D_80122C16 = (u16)count;
        return;
    }
    akao_set_song_params(0x8002, 0x2E, temp_s2, 0);
}
