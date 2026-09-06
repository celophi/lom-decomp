#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[0x1F];
    u8 unk20;
    u8 pad21[0x13];
    u32 unk34;
    u8 pad38[8];
} Rec800C9684;

extern u8 D_80122C1E;
extern s32 D_8011F428;
extern u8 D_80122C19[];
extern Rec800C9684 D_80122A08[];

void func_800A8F8C();

/**
 * @brief Apply the pending field mode transition to the four shared records.
 */
void func_800C94F4(void)
{
    s32 mode;
    s32 i;
    u8 *entry;
    s32 result;

    mode = D_80122C1E;
    if (mode == 1 && D_8011F428 == 0)
    {
        for (i = 0; i < 4; i++)
        {
            if (D_80122A08[i].unk0 == 0)
            {
                entry = &D_80122C19[i];
                if (D_80122A08[i].unk34 != 0 && *entry != 0xFA)
                {
                    D_80122A08[i].unk0 = D_80122A08[i].unk20;
                    D_80122A08[i].unk20 = *entry;
                    if (func_800A9060(entry) != 0)
                    {
                        func_800A8F8C(func_800A9060(), &D_80122A08[i]);
                    }
                }
            }
        }
    }
    if (mode == 0 && D_8011F428 == 1)
    {
        for (i = 0; i < 4; i++)
        {
            if (D_80122A08[i].unk0 != 0)
            {
                result = D_80122A08[i].unk34;
                if (result == 0)
                {
                    result = 1;
                }
                D_80122A08[i].unk34 = result;
            }
        }
    }
    D_80122C1E = (u8) D_8011F428;
}
