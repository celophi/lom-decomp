#include "common.h"

extern void func_80019788(s32);
extern void func_80019C74(void*, s32);
extern void func_80052458(s32, void*);
extern void func_80054B1C(void);

extern u8 D_801ED804;
extern unsigned int D_801ED02C;

typedef struct
{
    u16 unk0;
    u16 unk2;
    u8 pad[12];
    u32 unk10;
} SomeStruct;

/**
 * decomp.me (100%) https://decomp.me/scratch/m1WWc
 */
void func_80051F28(void* arg0, unsigned short arg1)
{
    u32* mem;
    s32 zero = 0;
    func_80019788(zero);
    func_80019C74(arg0, 0x1010);
    func_80019C74(((char*)arg0) + 0x7CC4, 0x1010);
    func_80052458(arg1 & 0xFFFF, arg0);
    mem = (u32*)0x801ED000;
    mem[1] = mem[0];
    mem[0] = mem[0] + 0x60;
    func_80054B1C();
    *((u32*)(((char*)arg0) + 0x40B8)) = mem[3];
    *((u32*)(((char*)arg0) + 0xBD7C)) = mem[4];
}

/**
 * decomp.me (100%) https://decomp.me/scratch/S4vVP
 */
void func_80051FBC(void)
{
    SomeStruct* ptr = (SomeStruct*)0x801ED480;
    ptr->unk0 = 0;
    ptr->unk2 = 0;
    ptr->unk10 = 0;
    D_801ED02C = 0;
    func_800642D4();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/lg9gw
 */
void func_80051FF8(s32 unused, s32 base, s32 arg2, s32 arg3)
{

    u8* struct_ptr;
    if (arg3 != 0)
    {
        func_80054CA8(base + 0x40B8, base + 0x40, 2);
    }
    else
    {
        func_80054CA8(base + 0x40B8, base + 0x40, arg2);
    }
    struct_ptr = (u8*)0x801ED800;
    func_80059C44();
    if (D_801ED804 != 0)
    {

        func_80140D48();
    }
    func_80064C28(base + 0x40B8, base, arg2);
    if (struct_ptr[4] != 0)
    {
        func_80140D48();
    }
}