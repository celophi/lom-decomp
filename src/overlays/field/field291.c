#include "common.h"

extern u8 *D_80122B74;

void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
void func_800C3A00(s32 arg0);
void func_8006AB38(s32 arg0);
void func_800C32C8(void);

void func_800C31BC(s32 arg0)
{
    if (arg0 == 0)
    {
        if ((*(s32 *)(D_80122B74 + 0x858) & 0x7F) == 2)
        {
            func_800BD520(0, (D_80122B74[0x859] << 3) + 0xF87, 0);
        }
        func_800BD520(0, 0x2F08, 0xFF);
        D_80122B74[0x840] = 0;
        *(s32 *)(D_80122B74 + 0x858) |= 0x7F;
    }
    else
    {
        if ((*(s32 *)(D_80122B74 + 0xAA8) & 0x7F) == 3)
        {
            func_800C32C8();
            *(s32 *)(D_80122B74 + 0x2EF0) = 5;
        }
        else
        {
            func_800C3A00(0);
        }
        D_80122B74[0xA90] = 0;
        *(s32 *)(D_80122B74 + 0xAA8) |= 0x7F;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    func_8006AB38(arg0);
}
