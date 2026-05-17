#include "common.h"

extern void func_80019788(s32);
extern void func_80019C74(void*, s32);
extern void func_80052458(s32, void*);
extern void func_80054B1C(void);

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