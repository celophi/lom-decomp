#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct Packet Packet;
struct Packet
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ void (*unk8)();
};

typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ u8 unk4[8];
    /* 0xC */ s32 unkC;
} CdStreamCtrl;

extern s32 D_80122988;
extern s32 D_80146918;
extern CdStreamCtrl D_801468B8;
extern s32 D_8014A920;
extern u8 D_8014A988[];
extern s16 D_8014EA38;
extern s32 D_8015A310;
extern s32 D_8015A318;
extern s32 D_8015A320;
extern s32 D_8015A324;
extern s32 D_8015A328;
extern u8 D_8015A350[];
extern s32 D_80162350;
extern s32 D_80162354;
extern s32 D_80162358;
extern s32 D_8016235C;
extern s32 D_80162364;
extern s32 D_80162368;
extern s32 D_80162370;
extern s32 D_801468C4;
extern u8 D_8014651C[];
extern u8 D_8014652C[];
extern u8 D_80146534[];
extern u8 *D_80162374;
extern s32 D_8003EC9C;
extern char D_800ECF7C[];
extern char D_801623D0[];
extern u8 D_80162C5F;
extern s32 D_80162E20;

extern int strncmp(char *, char *, int);
void func_801428DC();

/**
 * @see decomp.me (100%) TODO
 */
s32 func_801400C4(void)
{
    RECT rect;

    D_80162350 = 0xFF;
    D_8014A920 = 0;
    func_80143DE4();
    func_80143324();
    func_8014033C();
    D_80162358 = 0;
    func_8014485C();
    D_8015A328 = 0;
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    func_80145CD8();
    D_80162370 = 0;
    D_8015A320 = 0;
    D_80162364 = 0;
    D_8015A310 = 0;
    D_80162368 = 0;
    D_80146918 = 0;
    func_800AA02C();
    func_801404D4();
    func_8014019C();
    return D_80162358;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8014019C(void)
{
    RECT rect;
    u8 *p;
    u8 *q;
    s32 flag;
    s32 temp;

    func_80019788(0);
    func_8002054C(0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    func_8001990C(&rect, 0, 0, 0);
    p = D_8014A988;
    flag = 0;
    func_80019C74(p + 0x40, 0x1000);
    func_80019C74(p + 0x7D04, 0x1000);
    func_80019FB8(p + 0x4040);
    func_800157DC();
    func_800196F0(1);
    do
    {
        q = p + 0x40;
        func_80019C74(q, 0x1000);
        *(u8 **)(p + 0x40B8) = D_8015A350 + (flag << 14);
        func_800A9E78();
        temp = D_80122988 & 0xF000;
        if (temp != 0)
        {
            D_80122988 = temp;
        }
        func_80067BBC(p);
        if (func_80140448(p) != 0)
        {
            break;
        }
        func_80019788(0);
        func_800157B0(2);
        func_8002054C(2);
        func_8001990C(p + 0x40B0, 0, 0, 0);
        flag = 0;
        if (p == D_8014A988)
        {
            p += 0x7CC4;
            flag = 1;
        }
        else
        {
            p = D_8014A988;
        }
        func_80019FB8(p + 0x4040);
        func_80019DEC(p + 0x4054);
        func_80019D7C(q + 0x3FFC);
        func_800157DC();
        func_800122C0();
    } while (1);
    func_800158E0();
    func_8002054C(0);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8014033C(void)
{
    u8 *base;
    s16 *bank2;

    /* Reserve the outgoing-argument area: a wider (7-arg) call was compiled
       out here, so the frame keeps its space. */
    if (0)
    {
        func_8002054C(0, 0, 0, 0, 0, 0, 0);
    }
    func_8001D5AC(0x5DC);
    func_8001D58C(0xA0, 0x78);
    D_8014EA38 = 0;
    base = (u8 *)&D_8014EA38;
    bank2 = (s16 *)(base + 0x7CC4);
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0x4) = 0x140;
    *(s16 *)(base + 0x6) = 0xF0;
    *(s16 *)(base + 0x7CC4) = 0;
    bank2[1] = 0xE8;
    bank2[2] = 0x140;
    bank2[3] = 0xF0;
    func_80019788(0);
    func_8002054C(0);
    func_8001C62C(base - 0x70, 0, 0, 0x140, 0xF0);
    func_8001C62C(base + 0x7C54, 0, 0xE8, 0x140, 0xF0);
    func_8001C56C(base - 0x5C, 0, 0xF0, 0x140, 0xE0);
    func_8001C56C(base + 0x7C68, 0, 0x8, 0x140, 0xE0);
    base[0x7C7E] = 0;
    base[-0x46] = 0;
    func_80067B8C();
    func_80067EB4(0x100, 0x100, 0x100, 0x14);
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_80140448(s32 arg0)
{
    if (D_80146918 != 0)
    {
        func_80144A38();
        func_800643E0();
        func_80019788(0);
        return 1;
    }
    func_8006441C();
    func_80145C5C();
    func_801407BC(arg0);
    func_80145C98();
    func_80063194();
    D_80162368 ^= 1;
    return 0;
}

void func_80140DA4();
void func_80141474();
void func_801414D0();
void func_80141544();
void func_801415B8();
void func_80141CD0();
Packet *func_80141D04();

/**
 * @brief Build the fixed 5-entry GPU primitive/callback chain for the loader.
 * @note WIP. Structure, control flow, and the FRAME-04 dead-call frame padding
 *       are correct; the residual is a register-allocation permutation - the
 *       value temp lands in v1 (target v0) and the five hoisted mask constants
 *       occupy a permuted set of saved registers.
 * @see decomp.me (83.09%) TODO
 */
void func_801404D4(void)
{
    Packet *p;
    s32 v;
    s32 v1;

    D_8016235C = 0;
    D_8015A324 = 0;
    D_8015A318 = 0;
    D_80162354 = 0;
    D_80162364 = 0;
    if (0)
    {
        func_80141CD0(0, 0, 0, 0, 0);
    }
    func_80141CD0();
    v = D_801468B8.unk0;
    v = v & ~7;
    v = v | 1;
    D_801468B8.unk0 = v;

    p = func_80141D04();
    p->unk8 = func_80140DA4;
    v = p->unk0;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0xE00;
    p->unk0 = v;
    *((u8 *)p + 2) = 0x4A;
    v = p->unk4;
    v = v & ~0x200;
    p->unk4 = v;
    v1 = p->unk0;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x08000000;
    p->unk0 = v1;
    v = v | 1;
    v = v & ~0x1FE;
    v = v | 0x92;
    p->unk4 = v;

    p = func_80141D04();
    p->unk8 = func_80141474;
    v = p->unk0;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0x2800;
    p->unk0 = v;
    *((u8 *)p + 2) = 0xC;
    v = p->unk4;
    v = v & ~0x200;
    p->unk4 = v;
    v1 = p->unk0;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0xA0000000;
    p->unk0 = v1;
    v = p->unk4;
    v = v & ~1;
    v = v & ~0x1FE;
    v = v | 0x1E;
    p->unk4 = v;

    p = func_80141D04();
    v = p->unk0;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0xC00;
    p->unk0 = v;
    p->unk8 = func_801414D0;
    *((u8 *)p + 2) = 0x2C;
    v = p->unk4;
    v = v & ~0x200;
    p->unk4 = v;
    v1 = p->unk0;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x80000000;
    p->unk0 = v1;
    v = v & ~1;
    v = v & ~0x1FE;
    v = v | 0x1E;
    p->unk4 = v;

    p = func_80141D04();
    v = p->unk0;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0x5400;
    p->unk0 = v;
    p->unk8 = func_80141544;
    *((u8 *)p + 2) = 0x2C;
    v = p->unk4;
    v = v & ~0x200;
    p->unk4 = v;
    v1 = p->unk0;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x80000000;
    p->unk0 = v1;
    v = v & ~1;
    v = v & ~0x1FE;
    v = v | 0x1E;
    p->unk4 = v;

    p = func_80141D04();
    p->unk8 = func_801415B8;
    v = p->unk0;
    v = v & ~7;
    v = v | 2;
    v = v & ~0x78;
    v = v | 8;
    v = v & 0xFFFF007F;
    v = v | 0xF00;
    p->unk0 = v;
    *((u8 *)p + 2) = 0xA0;
    v = p->unk4;
    v = v & ~0x200;
    p->unk4 = v;
    v1 = p->unk0;
    v1 = v1 & 0xFFFFFF;
    v1 = v1 | 0x04000000;
    p->unk0 = v1;
    v = v | 1;
    v = v & ~0x1FE;
    v = v | 0x66;
    p->unk4 = v;

    v = D_801468B8.unk0;
    v = v & ~7;
    D_801468B8.unk0 = v;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_801407BC(void)
{
    s32 delta;

    func_80140D84();
    D_8015A328 += 2;
    if ((D_801468C4 & 0x7F) == 2)
    {
        func_801408A4();
    }
    if ((u16)D_80122988 == 0xFFFF)
    {
        D_80122988 = 0;
    }
    func_80140964();
    if (D_8016235C != 0)
    {
        s32 base = D_8015A318;
        delta = (D_8015A324 - D_8015A318) / D_8016235C;
        D_8016235C -= 1;
        D_8015A318 += delta;
    }
    else
    {
        D_8015A318 = D_8015A324;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_801408A4(s32 arg0)
{
    if (D_80162350 >= 0x10)
    {
        if (D_80162374 == NULL)
        {
            D_80162374 = D_8014651C;
        }
    }
    do
    {
        arg0 = func_8014401C(arg0);
    } while (arg0 == 3);
    if (arg0 == 2)
    {
        D_80162374 = D_80146534;
    }
    if (arg0 == 4)
    {
        D_80162374 = D_8014652C;
    }
    if (arg0 == 5)
    {
        D_80162350 = 0xF9;
        D_80162374 = D_80146534;
    }
}

/**
 * @note WIP. Structurally and semantically correct (verified via probe:
 *       the packet unk0/unk4 register roles and the loop-preheader a1/a2
 *       roles are a coupled register-coloring pair - fixing one region's
 *       roles regresses the other by the same amount). Every attempt to
 *       force the target's exact roles, including a permuter candidate
 *       that scored higher, was verified to silently drop the `| 0x56`
 *       term from the packet unk4 store and rejected.
 * @see decomp.me (94.25%) TODO
 */
s32 func_80140964(void)
{
    s32 pending;
    s32 status;
    s32 flag_a3;
    s32 flag_a2;
    s32 count;
    s32 last;
    s32 arg0;
    Packet *p;

    if ((D_801468B8.unkC & 7) == 0)
    {
        D_80146918 = 1;
        return;
    }
    if (D_80146918 != 0)
    {
        return;
    }
    if ((D_801468B8.unkC & 7) >= 3)
    {
        return;
    }
    if ((D_801468B8.unk0 & 7) != 0)
    {
        return;
    }
    pending = D_80162350;
    if (pending == 0xFF)
    {
        return;
    }
    if (D_80162E20 != 0)
    {
        return;
    }
    if (D_8015A310 != 0)
    {
        return;
    }
    if ((u32)(*D_80162374 - 6) < 2U)
    {
        return;
    }
    status = D_80122988;
    if (status & 0x40)
    {
        D_80146918 = 1;
        D_80162358 = 1;
        func_800A3938(0x78, 0x80);
        return;
    }
    if (status & 0xA100)
    {
        func_800A3938(0x7D, 0x80);
        D_8016235C = 0;
        D_8015A324 = 0;
        D_8015A318 = 0;
        D_80162354 = 0;
        D_80162374 = NULL;
        D_80162350 = 0xFF;
        D_80162364 = 0;
        D_8014A920 ^= 1;
        func_80143DE4();
        return;
    }
    if (pending >= 0x10)
    {
        return;
    }
    count = 1;
    if (status & 8)
    {
        D_80122988 = 0x4000;
        count = 1;
    }
    if (D_80122988 & 4)
    {
        D_80122988 = 0x1000;
        count = 1;
    }
    last = pending - 1;
    flag_a3 = D_80122988 & 0x1000;
    flag_a2 = D_80122988 & 0x4000;
    while (count != 0)
    {
        if (flag_a3 != 0)
        {
            D_80162354 -= 1;
            if (D_80162354 < 0)
            {
                D_80162354 = last;
            }
        }
        if (flag_a2 != 0)
        {
            D_80162354 += 1;
            if (D_80162354 >= pending)
            {
                D_80162354 = 0;
            }
        }
        count -= 1;
    }
    if (D_80122988 & 0x5000)
    {
        func_80144D18();
        func_800A3938(0x7D, 0x80);
        func_80140D20();
        return;
    }
    if (D_80122988 & 0x220)
    {
        s32 term1 = D_8014A920 * 0x320;
        s32 term2 = (D_80162354 * 0x28) + (s32)D_801623D0;

        if (strncmp(D_800ECF7C, (char *)(term1 + term2), 0xC) != 0)
        {
            arg0 = 0x78;
        }
        else
        {
            if ((D_80162C5F == D_8003EC9C) || (D_80162C5F == 0xFF))
            {
                p = func_80141D04(D_80162C5F);
                p->unk8 = func_801428DC;
                p->unk0 = (p->unk0 & ~0x78) | 8;
                p->unk0 = (p->unk0 & 0xFFFF007F) | 0x800;
                *((u8 *)p + 2) = 0x5B;
                p->unk4 = (p->unk4 | 1) & ~0x1FE;
                p->unk4 = p->unk4 | 0x56;
                p->unk0 = (p->unk0 & 0xFFFFFF) | 0x20000000;
                func_80143510();
                func_801447B4();
                arg0 = 0x7E;
            }
            else
            {
                arg0 = 0x78;
            }
        }
        func_800A3938(arg0, 0x80);
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80140CD0(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    var_a0 = (s32 *)&D_801468B8;
    var_a1 = 0;
    do
    {
        temp_v1 = *var_a0;
        if (temp_v1 & 7)
        {
            temp = (temp_v1 & ~7) | 3;
            *var_a0 = (temp & ~0x78) | 0x40;
        }
        var_a1 += 1;
        var_a0 += 3;
    } while (var_a1 < 8);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80140D20(void)
{
    s32 base;
    s32 delta;

    base = D_80162354 * 14;
    delta = base - D_8015A318;
    if (delta >= 0x3C)
    {
        D_8015A324 = base - 0x38;
        D_8016235C = 4;
    }
    if (delta < 0)
    {
        D_8015A324 = D_80162354 * 14;
        D_8016235C = 4;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80140D84(void)
{
    func_80141D78();
}
