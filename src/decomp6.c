#include "decomp6.h"


void InitializeControllers(s8 arg0)
{
    u8* base;
    u8** new_var;
    u8* ptr;
    u8* new_var2;
    int new_var3;
    u16 temp;
    int i;
    int sentinel;
    unsigned int val_ff;
    int val_40;
    int a2;
    int a1;
    func_80030DF8((void*)0x801ED75C, (void*)0x801ED77E);
    D_801ED7A4 = VSyncCallback(0);
    base = (u8*)0x801ED600;
    base[0xAD] = 0;
    base[0x15B] = 0x10;
    func_80015708(base);
    func_80015708(base + 0xAE);
    func_80015708(base + 0x20);
    new_var2 = base + 0xCE;
    func_80015708(new_var2);
    i = 1;
    val_40 = 0x40;
    val_ff = 0xFF;
    sentinel = -1;
    ptr = base + 0xAE;
    do
    {
        temp = *((u16*)(ptr + 0x92));
        i--;
        ptr[0x94] = val_40;
        ptr[0x97] = 0;
        ptr[0x96] = 0;
        ptr[0x95] = 0;
        ptr[0x90] = arg0;
        ptr[0x91] = 0;
        ptr[0xAA] = 0;
        ptr[0xAB] = 0;
        ptr[0xAC] = 0;
        ptr[0x20] = val_ff;
        ptr[0x00] = val_ff;
        temp &= 0xF0FF;
        *((u16*)(ptr - -0x92)) = temp;
        ptr[0x92] = 0;
        ptr -= 0xAE;
    } while (new_var3 = i != sentinel);
    base[0x1A8] = 0 * 0;
    base[0x1A9] = 0;
    base[0x1A0] = 0;
    base[0x1A1] = 0;
    base[0x1AA] = 0;
    func_8002E958(val_ff, i, sentinel, val_40);
    do
    {
        VSync(0);
        func_80015674();
        a2 = 1;
        a1 = a2;
        ptr = base + 0xAE;
        do
        {
            temp = *((u16*)((*(new_var = &ptr)) + 0x92));
            if ((!(((temp >> 6) >> 2) & 1)) && (((temp >> 9) & 3) != 2))
            {
                a2 = 0;
            }
            a1--;
            ptr -= 0xAE;
        } while (a1 != (-1));
    } while (a2 == 0);
    base[0x1A2] = 0;
    base[0x1A3] = 0;
}
