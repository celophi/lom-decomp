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

extern Rec800C9684 D_80122A08[];

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C00;
extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

/** @brief Dispatch the nonempty menu record at buffer offset 0x840. */
void func_800C9404(void)
{
    D_80122C00 = g_menuLayoutBuffer[0x840];
    if (D_80122C00 != 0)
    {
        func_800B2844(0, &g_menuLayoutBuffer[0x840], 0xFF);
    }
}

extern u8 D_80122C1F;

/** @brief Count empty entries in the four-record menu table. */
void func_800C9448(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 4; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0x3160] == 0)
        {
            count++;
        }
    }
    D_80122C1F = (u8) count;
}


/** @brief Count empty entries in the hundred-record equipment table. */
void func_800C9488(void)
{
    s32 i;
    s32 count;
    u8 *p;

    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0xCE0] == 0)
        {
            count++;
        }
    }
    D_80122C1F = (u8) count;
}



/** @brief Clear the four shared records and their saved result words. */
void func_800C94C8(void)
{
    ((u8 *)D_80122A08)[0] = 0;
    ((u8 *)D_80122A08)[0x40] = 0;
    ((u8 *)D_80122A08)[0x80] = 0;
    ((u8 *)D_80122A08)[0xC0] = 0;
    *(s32 *)&((u8 *)D_80122A08)[0x34] = 0;
    *(s32 *)&((u8 *)D_80122A08)[0x74] = 0;
    *(s32 *)&((u8 *)D_80122A08)[0xB4] = 0;
    *(s32 *)&((u8 *)D_80122A08)[0xF4] = 0;
}

extern u8 D_80122C1E;
extern s32 D_8011F428;
extern u8 D_80122C19[];

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

extern u8 D_80043818;
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/** @brief Set field state 0x2F08 from the pending mode flag. */
void func_800C963C(void)
{
    if (D_80043818 != 0)
    {
        func_800BD520(0, 0x2F08, 0x80);
    }
    else
    {
        func_800BD520(0, 0x2F08, 0xFF);
    }
}

extern u8 D_80122C02;

/** @brief Save the selected shared record and install its pending result. */
void func_800C9684(void)
{
    s32 index;
    Rec800C9684 *rec;
    u8 tmp;
    u8 tmp2;

    index = D_80122C02;
    tmp = D_80122A08[index].unk20;
    rec = &D_80122A08[index];
    (&D_80122C02)[index + 0x17] = tmp;
    tmp2 = rec->unk0;
    rec->unk0 = 0;
    rec->unk20 = tmp2;
    rec->unk34 = *(u32 *) ((u8 *) &D_80122C02 + 6);
}
