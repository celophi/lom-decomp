#include "common.h"

extern void func_80019788(s32);
extern void func_80019C74(void*, s32);
extern void func_80052458(s32, void*);
extern void func_80054B1C(void);

extern u8 D_801ED804;
extern unsigned int D_801ED02C;
extern s32 D_801ED000;
extern s32 D_801ED004;
extern s32 D_801ED010;
extern s32 D_801ED00C;
extern u16 D_801ED480;
extern u16 D_801ED482;

typedef struct
{
    u16 unk0;
    u16 unk2;
    u8 pad[12];
    u32 unk10;
} SomeStruct;

/* Node structure used in the linked list */
typedef struct Node
{
    struct Node* unk0; // offset 0x00 (next pointer)
    u8 pad[32];
    u32 unk24;
    u32 unk28;
    u32 unk2C;
    u32 unk30;
} Node;

/* Structure for the global pointer D_80180014 */
typedef struct
{
    u8 padding[8]; // offsets 0x00-0x07 (unknown/unused)
    Node* unk8;    // offset 0x08 (pointer to head of list)
} D_80180014_t;

extern D_80180014_t* D_80180014;
extern void func_80056A04(void); /* extern */

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

/**
 * decomp.me (100%) https://decomp.me/scratch/KyLZb
 */
void func_800520A0(s32 arg0, s32 arg1)
{
    Node* var_v0;

    var_v0 = D_80180014->unk8;
    if (var_v0 != 0)
    {
        do
        {
            var_v0->unk24 = 0;
            var_v0->unk28 = 0;
            var_v0->unk2C = 0;
            var_v0->unk30 = 0;
            var_v0 = var_v0->unk0;
        } while (var_v0 != 0);
    }
    if ((arg0 == 0) && (arg1 == 0))
    {
        func_80056A04();
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/EXpXm
 */
void func_80052108(void* unused, void* arg1)
{
    u16 temp_s1;
    u16 first_val;
    u8* addr = (u8*)0x801ED480;

    func_80019788(0);
    func_800119C0(0xB, (void*)0x80140000);
    func_80140018(0);

    // Read two 16-bit values from fixed address 0x801ED480
    first_val = *(u16*)addr;
    func_800522B4(first_val);
    temp_s1 = *(u16*)(addr + 2);

    func_80019788(0);
    func_80019C74(arg1, 0x1010);
    func_80019C74((void*)((char*)arg1 + 0x7CC4), 0x1010);
    func_80052458(temp_s1 & 0xFFFF, arg1);

    D_801ED004 = D_801ED000;
    D_801ED000 += 0x60;

    func_80054B1C();

    // Write to offsets 0x40B8 and 0xBD7C of the structure pointed by arg1
    *(s32*)((char*)arg1 + 0x40B8) = D_801ED00C;
    *(s32*)((char*)arg1 + 0xBD7C) = D_801ED010;

    func_800643E0();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/KMYoZ
 */
void func_800521DC(void)
{
    u16 temp_s1;
    void* temp_s0;
    char* ptr;

    temp_s0 = func_80015C28();
    func_80019788(0);
    func_800119C0(0xB, (void*)0x80140000);
    func_80140018(0);
    func_800522B4(D_801ED480);
    temp_s1 = D_801ED482;
    func_80019788(0);
    func_80019C74(temp_s0, 0x1010);
    func_80019C74((void*)((char*)temp_s0 + 0x7CC4), 0x1010);
    func_80052458(temp_s1 & 0xFFFF, temp_s0);
    D_801ED004 = D_801ED000;
    D_801ED000 += 0x60;
    func_80054B1C();

    ptr = (char*)temp_s0;
    *(s32*)(ptr + 0x40B8) = D_801ED00C;
    ptr += 0x8000;
    *(s32*)(ptr + 0x3D7C) = D_801ED010;

    func_800643E0();
}