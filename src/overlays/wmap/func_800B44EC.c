/* Partial WMAP decompilation: 95.784320% (gcc280_g0). */
#include "common.h"
extern u8 D_80121538;
extern void* D_80139280;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern s32 D_801B3040;
extern s32 D_801B3044;
extern void func_800B5BBC(void);
void func_800B44EC(void)
{
    s16* value;
    s32 index;
    s32 current;
    s32 setting;
    s32* entry;
    u8* base;
    u8* resource;
    void* state;
    index = 0x14;
    resource = &D_80121538;
    base = &D_80139988;
    entry = (s32*)(base + 0xA0);
    base = &D_801AFBD0;
    value = (s16*)(base + 0x190);
    do
    {
        *value = 0;
        entry[1] = (s32)resource;
        entry += 2;
        index++;
        value += 0xA;
    } while (index < 0x78);
    D_801B3044 = 0xB4;
    *(s32*)((u8*)D_80139280 + 0x7C) = setting = 5;
    *(s32*)((u8*)D_80139280 + 0x8C) = setting;
    state = D_80139280;
    current = D_801B3040;
    *(s32*)((u8*)state + 0x78) = 1;
    *(s32*)((u8*)state + 0x80) = -0x3E8;
    *(s32*)((u8*)state + 0x84) = 0x1770;
    *(s32*)((u8*)state + 0x88) = 0x1388;
    *(s32*)((u8*)state + 0x90) = -0x64;
    *(s32*)((u8*)state + 0x98) = -0x12C;
    *(s32*)((u8*)state + 0x94) = 0;
    *(s32*)((u8*)state + 0xA0) = -1;
    *(s32*)((u8*)state + 0xA4) = 0;
    D_801B3040 = current + 1;
    func_800B5BBC();
}
