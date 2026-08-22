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

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    /* 0x0 */ u32 tag;
    /* 0x4 */ u8 r0, g0, b0, code;
    /* 0x8 */ s16 x0, y0;
    /* 0xC */ s16 w, h;
} TILE;

extern s32 D_80122988;
extern s32 D_80146918;
extern CdStreamCtrl D_801468B8;
extern s32 D_8014A920;
extern u8 D_8014A988[];
extern s16 D_8014EA38;
extern s32 D_8015A310;
extern s32 D_8015A314;
extern s32 D_8015A318;
extern s32 D_8015A31C;
extern s32 D_8015A320;
extern s32 D_8015A324;
extern s32 D_8015A328;
extern s32 D_8015A330;
extern u8 D_8015A350[];
extern s32 D_80162350;
extern s32 D_80162354;
extern s32 D_80162358;
extern s32 D_8016235C;
extern s32 D_80162364;
extern s32 D_80162368;
extern s32 D_80162370;
extern s32 D_80162A10;
extern s32 D_80162A14;
extern u8 D_80162B90[];
extern s32 D_801468C4;
extern u8 D_8014651C[];
extern u8 D_8014652C[];
extern u8 D_80146534[];
extern u8 D_8014653C[];
extern s32 D_8014A924;
extern u8 *D_80162374;
extern s32 D_8003EC9C;
extern char D_800ECF7C[];
extern char D_801623D0[];
extern u8 D_80162C5F;
extern s32 D_80162E20;
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern u16 D_80145E9C;
extern u16 D_80145E9E;
extern u16 D_80145EA0;
extern u16 D_80145EA2;
extern u16 D_80145EA4;
extern u16 D_80145EA8;
extern u16 D_80145EAA;
extern u16 D_80145EAC;
extern u16 D_80145EAE;
extern u16 D_80145EB0;
extern u16 D_80145EC4;
extern u16 D_80145EC8;
extern u16 D_80145ECA;
extern u16 D_80145ECC;
extern u16 D_80145ECE;
extern u8 D_80146920[];
extern u8 D_80042FD8[];
extern s32 D_80042FB4;
extern s32 D_80162DC8;
extern s32 D_80162DD0;
extern s32 D_80162360;
extern u16 D_80145ED0;
extern u16 D_80145ED6;
extern u16 D_80145ED8;
extern u16 D_80145EDA;
extern u16 D_80145EDC;
extern u16 D_80145EDE;
extern u16 D_80145EF0;
extern u16 D_80145F4C;
extern s32 D_8016237C;
extern s32 D_80162380[];
extern s32 D_80162D78[];
extern u8 D_800EC3F6[2];
extern u16 D_80146338[];

extern int strncmp(char *, char *, int);
s32 func_801428DC();
s32 func_80142AD0(s32 ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80142E7C(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
u8 *func_80141428(void *);
void func_80141C6C(void *arg0);
s32 func_801430C8(s32 prim, s32 *ot, s32 x, s32 y, s32 highlight, s32 icon, s32 index, s32 row);
s32 func_80145764(s32 prim, s32 *ot, u8 *str, s32 x, s32 y, s32 a5, s32 a6);

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
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    func_80145C5C();
    func_801407BC(arg0);
    func_80145C98();
    func_80063194();
    D_80162368 ^= 1;
    return 0;
}

s32 func_80140DA4(s32 *, s32, s32, s32);
s32 func_80141474(s32 *, s32, s32, s32);
s32 func_801414D0(s32 *, s32, s32, s32);
s32 func_80141544(s32 *, s32, s32, s32);
s32 func_801415B8(s32 *, s32, s32, s32);
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

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

/**
 * @note WIP. Menu string/glyph-row drawing callback (state-dispatched TILE +
 *       text renderer). Structurally correct; residue is an `ot` parameter
 *       register-letter offset that recurs through most of the function body
 *       (content matches, register name differs - counted as argdiff, not a
 *       real mismatch) plus two small leftover items: a `D_80162350` reload
 *       at the loop's zero-trip guard that the target avoids by reusing the
 *       switch dispatch value, and one duplicated `D_80145EA4` address
 *       computation right before the final row's string-glyph call.
 * @see decomp.me (93.52%) TODO
 */
s32 func_80140DA4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 state = D_80162350;

    switch (state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145ED0, 0x34), 1, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145ED0, 0x34), 1, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145E9E, 2), 1, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EA0, 4), 1, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EAC, 0x10), 1, -arg2 + 0x84, -arg3, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EAE, 0x12), 1, -arg2 + 0x84, -arg3, 2);
        break;
    default:
        if (D_80162E20 != 0)
        {
            s32 x;
            u8 *base;
        case 0xFF:
            x = -arg2 + 0x84;
            base = (u8 *)&D_80145E9C;
            prim = func_800A88A0(prim, ot, base + D_80145E9C, 1, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 1, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 1, x, 0x1C - arg3, 2);
            break;
        }
        if (state > 0)
        {
            s32 i;
            s32 off;
            s32 base_x;
            s32 *flag_ptr;
            void *str_glyph;
            void *misc_glyph;
            char *entry;
            Vec2s pos;
            s32 row_y;
            u8 *base;

            base_x = -arg2;
            entry = D_801623D0;
            base = (u8 *)&D_80145E9C;
            for (i = 0, off = 0; i < D_80162350; entry += 0x28, i++, off += 4)
            {
                row_y = ((i * 14) - arg3) - D_8015A318;
                if ((u32)(row_y + 0xD) < 0x56U)
                {
                    flag_ptr = (s32 *)((u8 *)D_80162380 + off);
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A88A0(func_800A8A78(ot, prim, *(s32 *)((u8 *)D_80162D78 + off), 1, &pos, 0), ot, base + D_80145ECA, 1, base_x + 0x70, row_y, 0);
                        if ((D_8016237C - 1) == *flag_ptr)
                        {
                            misc_glyph = GLYPH_OFF(base, 0x36);
                            prim = func_800A88A0(prim, ot, misc_glyph, 1, base_x + 0xC2, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = GLYPH_OFF(base, 0x38);
                            prim = func_800A88A0(prim, ot, misc_glyph, 1, base_x + 0xC2, row_y, 0);
                        }
                        if (*func_80141428((void *)((D_8014A920 * 0x320) + (s32)entry + 0xC)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, base + D_80145F4C, 1, 0xF8 - arg2, row_y, 1);
                        }
                    }
                    if (strncmp(D_800ECF7C, (char *)((D_8014A920 * 0x320) + (s32)entry), 0xC) == 0)
                    {
                        str_glyph = base + D_80145EA2;
                    }
                    else if (strncmp(D_800ECF8C, (char *)((D_8014A920 * 0x320) + (s32)entry), 0xC) == 0)
                    {
                        str_glyph = base + D_80145ED6;
                    }
                    else if (strncmp(D_800ECFC4, (char *)((D_8014A920 * 0x320) + (s32)entry), 8) == 0)
                    {
                        str_glyph = base + D_80145EB0;
                    }
                    else
                    {
                        str_glyph = base + D_80145EA4;
                    }
                    prim = func_800A88A0(prim, ot, str_glyph, 1, base_x, row_y, 0);
                }
            }
        }
        {
            s32 y0 = ((D_80162354 * 14) - arg3) - D_8015A318;

            if (D_80162E20 == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = 0xF080F0;
                *((u8 *)tile + 3) = 3;
                tile->code = 0x62;
                tile->y0 = (s16)(y0 - 1);
                tile->w = 0x108;
                tile->x0 = 0;
                tile->h = 0xE;
                tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | ((s32)tile & 0xFFFFFF);
                prim += 0x10;
            }
        }
        break;
    }
    return prim;
}

/**
 * @brief Advance a pointer past a run of hex-digit characters ('0'-'9',
 *        'a'-'f', 'A'-'F').
 * @param arg0 Pointer to the first character to test.
 * @return Pointer to the first character that is not a hex digit.
 * @note WIP. Semantics, control flow, and the target bytes are fully
 *       understood (three independent range checks, each looping back to
 *       the function's own entry on a match). The residual is that GCC's
 *       jump2 cross-jump pass (jump.c, cross_jump=1) merges all three
 *       "advance and loop back" arms into one shared tail here, while the
 *       target keeps them independent - each of its three branches carries
 *       its own delay-slot increment plus a compensating decrement on the
 *       fallthrough (reorg target-steal-with-compensation). No source
 *       rephrasing tried so far (if/else-if, sequential if-continue,
 *       self-tail-recursion, do-while, for-loop-continue, precomputed
 *       conditions) avoids the merge; see the crossjump_oracle OVER-MERGED
 *       finding and idiom candidates JUMP-16/JUMP-19 in idioms.md, neither
 *       of which closed it. A 300k-iteration permuter run found no valid
 *       (semantics-preserving) improvement past this score.
 * @see decomp.me (65.63%) TODO
 */
u8 *func_80141428(void *arg0)
{
    u8 *p;
    u8 c;

    p = (u8 *)arg0;
    while (1)
    {
        c = *p;
        if ((u32)(c - '0') < 10)
        {
            p++;
            continue;
        }
        if ((u32)(c - 'a') < 6)
        {
            p++;
            continue;
        }
        if ((u32)(c - 'A') < 6)
        {
            p++;
            continue;
        }
        break;
    }
    return p;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_80141474(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    return func_800A88A0(prim, ot, GLYPH_SYM(D_80145EC8, 0x2C), 1, -arg2 + 0x50, -arg3, 2);
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_801414D0(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 a3;
    void *glyph;
    RECT pos;

    a3 = 1;
    glyph = GLYPH_SYM(D_80145EA8, 0xC);
    if (D_8014A920 != 0)
    {
        a3 = 3;
    }
    return func_800A88A0(prim, ot, glyph, a3, -arg2 + 0x40, -arg3, 2);
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_80141544(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 a3;
    void *glyph;
    RECT pos;

    a3 = 1;
    glyph = GLYPH_SYM(D_80145EAA, 0xE);
    if (D_8014A920 == 0)
    {
        a3 = 3;
    }
    return func_800A88A0(prim, ot, glyph, a3, -arg2 + 0x40, -arg3, 2);
}

/**
 * @note WIP. Save-slot HUD callback: draws either the elapsed-play-time
 *       display (hours:minutes plus a 3-memcard-icon highlight strip) when
 *       the slot name matches the empty-slot marker, or the slot's save-file
 *       name otherwise. Structurally and semantically matched (frame, control
 *       flow, and every field offset into the D_80162B90 status block agree
 *       with the target); the residue is a coupled register-allocation
 *       problem in the icon-highlight loop's func_801430C8 call: the target
 *       tracks the "entries seen so far" count and the raw loop index as two
 *       separate spilled locals (sp+0x18/0x1c, extra sp+0x138/0x140 spills
 *       around the call), while reintroducing that second counter here
 *       consistently regresses the whole function's register allocation
 *       instead of only affecting this call. Permuter (gcc272_cdk, ~60k
 *       iterations across two seeds) found no valid improvement past this
 *       point.
 * @see decomp.me (89.53%) TODO
 */
s32 func_801415B8(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[228];
    s32 slot[3];

    result = prim;
    if (D_80162364 == 0)
    {
        return result;
    }
    if (D_80162E20 != 0)
    {
        return result;
    }
    if (D_80162364 != 3 && D_80162350 < 0x10)
    {
        if (D_80162364 == 2)
        {
            s32 x = -arg2;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EC4, 0x28), 1, x, -arg3, 0);
            base = (u8 *)&D_80145EC4 - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 1, x, 0x10 - arg3, 0);
        }
        else
        {
            s32 term1 = D_8014A920 * 0x320;
            s32 term2 = (D_80162354 * 0x28) + (s32)D_801623D0;

            if (strncmp(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0)
            {
                u8 *base90 = D_80162B90;

                if (base90[0xCF] == 0xFF || base90[0xCF] == D_8003EC9C)
                {
                    s32 present_count;
                    s32 i;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 minutes;
                    s32 sign_adj;
                    s32 time_val;

                    slot[0] = (u32)(*(s32 *)(base90 + 0x18)) >> 0x19;
                    slot[1] = ((u32)(*(s32 *)(base90 + 0x20)) >> 0x12) & 0x7F;
                    slot[2] = (u32)(*(s32 *)(base90 + 0x20)) >> 0x19;
                    D_8015A31C = (s32)base90[0x1F];

                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = D_8015A328;
                        if (D_8015A328 < 0)
                        {
                            time_val = D_8015A328 + 0x1F;
                        }
                        D_8015A328 -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        D_8015A328 %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        D_8015A328 = 0x1F;
                        break;
                    }

                    total = 0;
                    base_x = half_step;
                    base_y = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((D_8015A328 >= base_y && D_8015A328 < base_x && (delta = D_8015A328 - base_y, 1))
                                || (rem = base_x % (step * present_count), D_8015A328 >= rem && D_8015A328 < (hi = rem + step) && (delta = hi - D_8015A328, 1)))
                            {
                                adjust += delta;
                            }
                            total += adjust;
                            result = func_801430C8(result, ot, total - arg2, -arg3, adjust, slot[i], i, i);
                        }
                        base_x += step;
                        base_y += step;
                    }

                    {
                        s32 unk30 = *(s32 *)(base90 + 0x30);
                        s32 x = -arg2;
                        s32 y = -arg3;

                        sign_adj = unk30 >> 0x1F;
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = unk30 / 216000;
                        result = func_800A88A0(func_800A8A78(ot, result, hours, 1, &pos, 1), ot, D_800EC3F6[0] + (D_800EC3F6[1] << 8) + ((s32)&D_800EC3F6 - 0x32), 1, x + 0x6F, y, 0);
                        minutes = ((unk30 / 3600) - sign_adj) - (hours * 0x3C);
                        if (minutes < 0xA)
                        {
                            pos.x = (s16)(x + 0x7D);
                            pos.y = (s16)y;
                            result = func_800A8A78(ot, result, 0, 1, &pos, 1);
                        }
                        pos.x = (s16)(x + 0x85);
                        pos.y = (s16)y;
                        result = func_800A88A0(func_800A88A0(func_800A8A78(ot, result, minutes, 1, &pos, 1), ot, base90, 1, x + 0x54, y + 0x10, 0), ot, GLYPH_OFF((u8 *)D_80146338, (*(s32 *)(base90 + 0x20) & 0x3FFFF) * 2), 1, x + 0x54, y + 0x20, 0);
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_80145EF0, 0x54), 1, -arg2, -arg3, 0);
                }
            }
            else
            {
                s32 j;

                func_80141C6C(&D_80162A14);
                if ((u32)(*((u8 *)&D_80162A10 + 0x24) - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = *((u8 *)&D_80162A14 + j);
                    }
                    name[j] = 0;
                    result = func_80145764(result, ot, name, -arg2, -arg3, 1, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = *((u8 *)&D_80162A10 + 0x24 + j);
                    }
                    name[j] = 0;
                    result = func_80145764(result, ot, name, -arg2, 0x10 - arg3, 1, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80141C6C(void *arg0)
{
    u8 *p;
    s32 i;

    p = (u8 *)arg0;
    i = 0;
    for (;;)
    {
        if (i >= 0x40)
        {
            return;
        }
        if (*p == 0)
        {
            while (i < 0x40)
            {
                *p = 0;
                i++;
                p++;
            }
            return;
        }
        if (*p >= 0x80)
        {
            p += 2;
            i += 2;
        }
        else
        {
            p += 1;
            i += 1;
        }
    }
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80141CD0(void)
{
    Packet *p;
    s32 i;

    p = (Packet *)&D_801468B8;
    for (i = 0; i < 8; i++)
    {
        p->unk0 &= ~7;
        p++;
    }
}

/**
 * @see decomp.me (100%) TODO
 */
Packet *func_80141D04(void)
{
    Packet *p;
    s32 i;

    p = (Packet *)&D_801468B8;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->unk0 & 7) == 0)
        {
            p->unk4 |= 0x200;
            p->unk0 = (p->unk0 & ~7) | 1;
            return p;
        }
    }
    return (Packet *)&D_801468B8;
}

/** @brief GPU packet emitted into the ordering table; unk0 is the OT link word. */
typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
    /* 0xC */ s16 unkC;
    /* 0xE */ u16 unkE;
} StructS0;

/** @brief Draw context at arg0 + 0x40; unk0 is the head of the ordering table. */
typedef struct
{
    /* 0x0 */ s32 unk0;
} RenderCtx;

/** @brief Owning screen state passed to the element tick/draw pass. */
typedef struct
{
    u8 pad0[0x40B2];
    /* 0x40B2 */ s16 unk40B2;
    u8 pad40B4[4];
    /* 0x40B8 */ StructS0 *unk40B8;
} Arg0Struct;

/** @brief Per-element draw callback stored at element + 8. */
typedef StructS0 *(*ElemDrawFunc)();

StructS0 *func_801427F8();
StructS0 *func_80142480();
StructS0 *func_801433EC();

/**
 * @brief Advance and draw the eight pool elements for one frame.
 *
 * Walks the 0xC-stride element pool at D_801468B8, links each live element's
 * packets into the ordering table, and dispatches on the low 3 bits of its
 * state word: 1 grows, 2 holds, 3 shrinks, 4 counts down to idle.
 *
 * @param arg0 Owning screen state; unk40B8 is the packet cursor, read on entry
 *             and written back on exit.
 *
 * @note NOT MATCHED (328/365 exact rows). Residue is the case 1/3/4 tails,
 *       where the target stores the updated state word before testing it and
 *       materializes the comparison constant 8 early; sched_oracle reports
 *       emit[li 8] < emit[andi 15] violated. Two shapes are required to match:
 *       `volatile u32 *var_s3` (the target emits 12 loads of the element word
 *       and gcc CSEs 3 away without it), and the `do { ... } while (0)` around
 *       the var_s0 advance, which is an [ALLOC-23] loop-note ref bump worth 28
 *       exact rows - it wins s0 for var_s0 against temp_s1. A second such
 *       wrapper around the case-1 state-word update is worth 2 more. gosub's
 *       func_80143C58 is the matched 100% twin and the source model to work
 *       from. See working/func_80141D78/STATUS.md.
 * @see decomp.me (96.49%) TODO
 */
void func_80141D78(Arg0Struct *arg0)
{
    StructS0 *var_s0;
    RenderCtx *var_s5;
    volatile u32 *var_s3;
    s32 temp_s1;
    s32 temp_s2;
    s32 sp80;
    s32 sp20[24];
    u32 temp_a3;
    u32 temp_a0_2;
    s32 temp_v1_2;
    u32 temp_a1;
    u32 temp_a2;
    s32 temp_a0_3;
    s32 var_v1;
    s32 temp_a3_2;
    s32 var_v0;
    s32 temp_a3_3;
    u32 temp_v0_3;
    u32 temp_a0_4;
    s32 temp_a0_5;
    s32 var_v1_2;
    s32 temp_a3_5;
    s32 var_v0_2;
    s32 temp_a3_6;
    u32 temp_v0_5;
    u32 temp_v1_3;

    var_s0 = arg0->unk40B8;
    var_s5 = (RenderCtx *)((u8 *)arg0 + 0x40);

    if ((D_80162350 < 0x10) && ((D_801468C4 & 7) == 2))
    {
        if ((D_80162350 * 0xE) > (D_8015A318 + 0x49))
        {
            var_s0 = func_801427F8(var_s0, var_s5, 0x114, 0x87, 0);
        }
        if (D_8015A318 != 0)
        {
            var_s0 = func_801427F8(var_s0, var_s5, 0x114, 0x4A, 1);
        }
    }

    if (arg0->unk40B2 != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s3 = (volatile u32 *)&D_801468B8;
    sp80 = 0;

    for (; sp80 < 8; sp80++, var_s3 += 3)
    {
        temp_a3 = *var_s3;
        if (temp_a3 & 7)
        {
            func_8001A5D4((s32)var_s0, sp20);

            var_s0->unk0 = (var_s0->unk0 & 0xFF000000) | (var_s5->unk0 & 0x00FFFFFF);
            var_s5->unk0 = (s32)((var_s5->unk0 & 0xFF000000) | ((s32)var_s0 & 0x00FFFFFF));

            temp_a0_2 = *var_s3;
            temp_v1_2 = temp_a0_2 & 7;

            do
            {
                var_s0 = (StructS0 *)((u8 *)var_s0 + 0x40);
            } while (0);

            switch (temp_v1_2)
            {
            case 1:
                temp_v0_3 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a2 = (temp_v0_3 >> 24) | ((temp_a1 & 1) << 8);
                temp_a0_3 = (temp_v0_3 >> 3) & 0xF;
                var_v1 = temp_a2 * temp_a0_3;
                D_80122988 = 0;
                if (var_v1 < 0)
                {
                    var_v1 += 7;
                }
                temp_a3_2 = (temp_a1 >> 1) & 0xFF;
                var_v0 = temp_a3_2 * temp_a0_3;
                temp_s1 = var_v1 >> 3;
                if (var_v0 < 0)
                {
                    var_v0 += 7;
                }
                temp_s2 = var_v0 >> 3;
                temp_a3_3 = (s32)(temp_a3_2 - temp_s2);

                var_s0 = (*(ElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = func_80142480(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, (*(u32 *)((u8 *)var_s3 + 4) >> 9) & 1);
                }
                do
                {
                    temp_a0_4 = *var_s3;
                    temp_a0_4 = (temp_a0_4 & ~0x78) | (((((temp_a0_4 >> 3) & 0xF) + 1) & 0xF) * 8);
                    *var_s3 = temp_a0_4;
                } while (0);
                if (((temp_a0_4 >> 3) & 0xF) == 8)
                {
                    func_800AA02C();
                    *var_s3 = (*var_s3 & ~7) | 2;
                }
                break;

            case 2:
                var_s0 = (*(ElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s3;
                    high = case_word >> 24;
                    var_s0 = func_80142480(var_s0, var_s5, (case_word >> 7) & 0x1FF, (*((u8 *)var_s3 + 2)),
                                           ((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF, arg0->unk40B2,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 9) & 1);
                }
                temp_v1_3 = *var_s3;
                if (((temp_v1_3 >> 3) & 0xF) != 0)
                {
                    *var_s3 = (temp_v1_3 & ~0x78) | (((((temp_v1_3 >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                temp_v0_5 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a2 = (temp_v0_5 >> 24) | ((temp_a1 & 1) << 8);
                temp_a0_5 = temp_v0_5 >> 3;
                temp_a0_5 = temp_a0_5 & 0xF;
                var_v1_2 = temp_a2 * temp_a0_5;
                D_80122988 = 0;
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 7;
                }
                temp_a3_5 = (temp_a1 >> 1) & 0xFF;
                var_v0_2 = temp_a3_5 * temp_a0_5;
                temp_s1 = var_v1_2 >> 3;
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 7;
                }
                temp_s2 = var_v0_2 >> 3;
                temp_a3_6 = (s32)(temp_a3_5 - temp_s2);

                var_s0 = (*(ElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = func_80142480(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, (*(u32 *)((u8 *)var_s3 + 4) >> 9) & 1);
                }
                temp_a0_4 = *var_s3;
                temp_a0_4 = (temp_a0_4 & ~0x78) | (((((temp_a0_4 >> 3) & 0xF) - 1) & 0xF) * 8);
                *var_s3 = temp_a0_4;
                if (!((temp_a0_4 >> 3) & 0xF))
                {
                    *var_s3 = (((temp_a0_4 & ~0x78) | 0x18) & ~7) | 4;
                }
                break;

            case 4:
                temp_v0_5 = *var_s3;
                D_80122988 = 0;
                temp_v1_3 = (temp_v0_5 & ~0x78) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *var_s3 = temp_v1_3;
                if (!((temp_v1_3 >> 3) & 0xF))
                {
                    *var_s3 = temp_v1_3 & ~7;
                }
                break;
            }
        }
    }

    arg0->unk40B8 = func_801433EC(var_s0, var_s5);
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8014232C(u8 *dest, u8 *src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = func_801423B0(dest);
    src_len = func_801423B0(src);
    for (i = 0; i < src_len; i++)
    {
        dest[dst_len + i] = src[i];
    }
    dest[dst_len + i] = 0;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_801423B0(u8 *arg0)
{
    u8 *p;
    u8 c;
    s32 len;

    p = arg0;
    c = *p;
    len = 0;
    while (c != 0)
    {
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
        c = *p;
    }
    return len;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_801423FC(u8 *dest, u8 *src)
{
    volatile u8 *p;
    s32 len;
    s32 i;

    p = (volatile u8 *)src;
    len = 0;
    while (*p != 0)
    {
        if ((u32)(*p - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
    }
    for (i = 0; i < len; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = 0;
}

StructS0 *func_8014269C();

/**
 * @note The `do { ... } while (0)` around the three func_8014269C calls is
 *       required to match; plain braces do not substitute.
 * @see decomp.me (100%) TODO
 */
StructS0 *func_80142480(StructS0 *prim, s32 *ot, s32 x, s32 y, s32 w, s32 h, s32 flag, s32 arg7)
{
    StructS0 *var_s0;
    StructS0 *var_a0;
    StructS0 *buf;
    StructS0 *result;
    s32 sp20[24];
    s32 temp_a2;

    buf = prim;
    if (flag != 0)
    {
        temp_a2 = y + 0xF2;
        func_8001C56C(sp20, x + 2, temp_a2, w - 4, h - 3);
    }
    else
    {
        temp_a2 = y + 0xA;
        func_8001C56C(sp20, x + 2, temp_a2, w - 4, h - 3);
    }
    func_8001A5D4((s32)buf, sp20);

    buf->unk0 = (buf->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)buf & 0xFFFFFF);

    result = (StructS0 *)((u8 *)buf + 0x40);
    if (arg7 != 0)
    {
        do
        {
            var_s0 = func_8014269C(result, ot, x, y, w, h, 0xFFFFFF);
            var_s0 = func_8014269C(var_s0, ot, x + 1, y + 1, w - 2, h - 2, 0);
            var_s0 = func_8014269C(var_s0, ot, x - 1, y - 1, w + 2, h + 2, 0);
        } while (0);

        result = var_s0;
        result->unk4 = 0x808080;
        ((u8 *)result)[3] = 3;
        ((u8 *)result)[7] = 0x62;
        result->unk8 = x;
        result->unkA = y;
        result->unkC = w;
        result->unkE = h;
        result->unk0 = (result->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)result & 0xFFFFFF);

        var_a0 = (StructS0 *)((u8 *)result + 0x10);
        ((u8 *)var_a0)[3] = 1;
        var_a0->unk4 = 0xE1000045;
        var_a0->unk0 = (var_a0->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)var_a0 & 0xFFFFFF);
        result = (StructS0 *)((u8 *)var_a0 + 8);
    }
    return result;
}

/**
 * @note `tmp` is deliberately reused for the packet-1 OT-link mask and then for
 *       the bottom edge's y coordinate. Both the mask binding and the reuse are
 *       required to match: with one basic block this function is allocated by
 *       local-alloc, and the reassignment truncates the constant's live range
 *       so it wins the lower temporary register ([ALLOC-23] style ref bumps do
 *       nothing here). Separate variables score 98.74%.
 * @see decomp.me (100%) TODO
 */
StructS0 *func_8014269C(StructS0 *p, s32 *ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    s32 tmp;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x;
    p->unkA = y;
    p->unkC = x + w;
    p->unkE = y;
    tmp = 0xFF000000;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & tmp) | ((s32)p & 0xFFFFFF);
    p++;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x + w;
    p->unkA = y;
    p->unkC = x + w;
    p->unkE = y + h;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
    p++;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x + w;
    tmp = y + h;
    p->unkA = tmp;
    p->unkC = x;
    p->unkE = y + h;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
    p++;

    p->unk4 = color;
    ((u8 *)p)[3] = 3;
    ((u8 *)p)[7] = 0x40;
    p->unk8 = x;
    p->unkA = y;
    p->unkC = x;
    p->unkE = y + h;
    p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
    return p + 1;
}

/**
 * @brief Ordering-table link word at the head of a GPU packet; mirrors the
 *        P_TAG layout in include/psyq/libgpu.h.
 * @note Writing the link through the 24-bit @c addr bitfield (setaddr) makes gcc
 *       build the two mask constants in the order the original used; the
 *       hand-written @c (x & 0xFF000000) | (y & 0xFFFFFF) form colours them the
 *       other way round.
 */
typedef struct
{
    /* 0x0 */ u32 addr : 24; /* next-primitive address (24-bit) */
    /* 0x3 */ u32 len : 8;   /* packet word count */
} OtTag;

/**
 * @see decomp.me (100%) TODO
 */
StructS0 *func_801427F8(StructS0 *p, s32 *ot, s32 x, s32 y, s32 flag)
{
    p->unk4 = 0x808080;
    do
    {
        ((u8 *)p)[3] = 4;
        ((u8 *)p)[7] = 0x64;
        p->unk8 = x;
        p->unkA = y;
    } while (0);
    if (flag != 0)
    {
        ((u8 *)p)[0xD] = 0;
    }
    else
    {
        ((u8 *)p)[0xD] = 0x10;
    }
    ((u8 *)p)[0xC] = 0x30;
    *(s16 *)((u8 *)p + 0x10) = 0x10;
    *(s16 *)((u8 *)p + 0x12) = 0x10;
    p->unkE = 0x7D80;
    ((OtTag *)p)->addr = ((OtTag *)ot)->addr;
    ((OtTag *)ot)->addr = (u32)p;

    p = (StructS0 *)((u8 *)p + 0x14);
    ((u8 *)p)[3] = 1;
    p->unk4 = 0xE100008A;
    ((OtTag *)p)->addr = ((OtTag *)ot)->addr;
    ((OtTag *)ot)->addr = (u32)p;
    return (StructS0 *)((u8 *)p + 8);
}

/**
 * @brief Draw the CD-load prompt glyph then set up the driver/GPU-packet state,
 *        branching on the CD status (func_80144804) and the D_80122988 flags.
 * @param ot ordering-table head threaded through the glyph/line draws.
 * @param prim primitive buffer for the glyph draw (func_800A88A0).
 * @param arg2 base for the row y-coordinate (-arg2 + 0x90).
 * @param arg3 row delta applied to the draw extents.
 * @return the StructS0* chain pointer returned by func_80143520.
 * @note WIP 96.72% (121/125 exact). Required-to-match shapes in place:
 *       FRAME-04 dead 9-arg call for the -0x38 outgoing-arg frame; ONE-EXIT
 *       `goto ret` single return so the `v0 = result` copy is shared; block-C
 *       laid out before block-B (target reaches C by branch, falls into B);
 *       D_801468B8 accessed dually (CdStreamCtrl `.unk0` direct via %hi/%lo +
 *       Packet* base for unk4/unk8/byte2); `tmp = 0xFFFF007F` bound before
 *       `p->unk8` (born before the base pointer, wins its register); the unk4
 *       update split as `tmp = (unk4 | 1) & ~0x1FE; unk4 = tmp | 0x56`.
 *       Residual (4 rows): the split unk4 accumulator lands in a0 where the
 *       target keeps it in v1, plus a 1-slot schedule shift on the
 *       -0x1FF/unk4-load pair. Coupled block-B coloring; permuter best 130.
 * @see decomp.me (96.72%) TODO
 */
s32 func_801428DC(s32 ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 result;
    s32 y;

    if (0)
    {
        func_80143520(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    y = -arg2 + 0x90;
    result = func_80143520(
        func_800A88A0(prim, (s32 *)ot, (void *)((s32)&D_80145ECC - 0x30 + D_80145ECC), 4, y, -arg3, 2),
        ot, y, 0xE - arg3);
    if ((u32)(func_80144804() - 1) < 2)
    {
        D_801468B8.unk0 = D_801468B8.unk0 & ~7;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        D_80162350 = 0xFF;
        func_80143DE4();
        D_80162374 = NULL;
        goto ret;
    }
    if (D_80122988 & 0x40)
    {
        goto block_c;
    }
    if (!(D_80122988 & 0x220))
    {
        goto ret;
    }
    if (D_8014A924 == 0)
    {
        goto block_b;
    }
block_c:
    D_801468B8.unk0 = D_801468B8.unk0 & ~7;
    func_800AA02C();
    func_800A3938(0x78, 0x80);
    D_80162374 = D_80146534;
    goto ret;
block_b:
    {
        Packet *p = (Packet *)&D_801468B8;
        s32 tmp;

        func_800A3938(0x7E, 0x80);
        D_8015A320 = 1;
        D_80162374 = D_8014653C;
        tmp = 0xFFFF007F;
        p->unk8 = func_80142AD0;
        D_801468B8.unk0 = (((((D_801468B8.unk0 & ~0x78) | 8) & ~7) | 1) & tmp) | 0x800;
        ((u8 *)p)[2] = 0x5B;
        tmp = (p->unk4 | 1) & ~0x1FE;
        p->unk4 = tmp | 0x56;
        D_801468B8.unk0 = (D_801468B8.unk0 & 0xFFFFFF) | 0x20000000;
    }
ret:
    return result;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_80142AD0(s32 ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 y;
    s32 result;
    u8 *base;
    u8 *p;
    s32 vol;

    if (0)
    {
        func_80142C58(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    y = -arg2 + 0x90;
    result = func_800A88A0(prim, (s32 *)ot, (void *)((s32)&D_80145ECE - 0x32 + D_80145ECE), 4, y, -arg3, 2);
    base = (u8 *)&D_80145ECE - 0x32;
    result = func_800A88A0(result, (s32 *)ot, GLYPH_OFF(base, 0x1E), 4, y, 0xE - arg3, 2);
    result = func_800A88A0(result, (s32 *)ot, GLYPH_OFF(base, 0xB2), 4, y, 0x1C - arg3, 2);
    result = func_80142C58(result, ot);

    if (D_8015A320 != 0)
    {
        goto ret;
    }

    p = D_80146920;
    vol = 0x80;
    base = p;
    if (func_80143648(base) == 0)
    {
        func_80142D68(4);
        goto ret;
    }

    func_800A3938(0x7B, vol);
    D_801468B8.unk0 = D_801468B8.unk0 & ~7;
    func_80016E7C(base + 0x180, D_80042FD8, 0x3268);
    D_8003EC9C = D_80042FD8[0xCF];
    D_80042FB4 = func_8002054C(-1);
    D_80146918 = 1;
ret:
    return result;
}

/**
 * @brief Same word/half raw-store granularity as POLY_G4 in
 *        include/psyq/libgpu.h; the r/g/b/code quad and each x/y pair are
 *        written as single word/half stores (matching the target's
 *        codegen), not per-channel byte assignments.
 */
typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
    /* 0xC */ s32 unkC;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
} PolyG4Words;

/**
 * @brief Build the CD-load progress-bar gouraud quad (a POLY_G4-shaped
 *        packet) and link it into the ordering table.
 * @param p packet cursor to write the quad into.
 * @param ot ordering-table head threaded through the OT-link update.
 * @return the advanced packet cursor (p + 0x24), or p unchanged if
 *         D_80162DC8 is 0.
 * @note WIP 94.26% (60/68 exact). D_80162DC8 gates the whole body; when set,
 *       `elapsed` (VSync(-1) - D_80162DD0, clamped to 0x100) scales into the
 *       quad's right-edge x extent (elapsed * 0x120, rounded via +0xFF for
 *       negative values, then >>8) - a time-based progress bar. Required-to-
 *       match shape: the two `>>8` stores are written inline
 *       (`g->unk20 = extent >> 8; g->unk10 = extent >> 8;`), NOT through a
 *       reassigned `extent = extent >> 8;` local - the reassignment costs
 *       -7 exact rows (this single change was the jump from 82.28% to
 *       94.26%, and incidentally fixed a second, separate-looking mismatch
 *       around the OT-link mask too).
 *       Residual (8 rows, SCHED-LUID): the target fills the elapsed-clamp
 *       branch's delay slot with the start of the `g->unk14 = 0xFFFF00`
 *       constant build (`lui`); this compile fills it by speculatively
 *       re-running the `x * 9` shift with the pre-clamp value and redoing it
 *       after the join (+1 insn). Named-local hoists of the 0xFFFF00/0xFFFFFF
 *       constants to various earlier points all measured neutral or worse;
 *       permuter (v2, ~16k iters) got to score 245 (from 575) via the same
 *       inline->8 fix already applied here and did not find the remaining
 *       delay-slot swap.
 * @see decomp.me (94.26%) TODO
 */
StructS0 *func_80142C58(StructS0 *p, s32 *ot)
{
    PolyG4Words *g;
    s32 elapsed;
    s32 extent;

    g = (PolyG4Words *)p;
    if (D_80162DC8 != 0)
    {
        elapsed = func_8002054C(-1) - D_80162DD0;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        extent = elapsed * 0x120;
        g->unk4 = 0xFF;
        g->unkC = 0xFFFF;
        g->unk1C = 0xFF0000;
        ((u8 *)g)[3] = 8;
        g->unk14 = 0xFFFF00;
        ((u8 *)g)[7] = 0x38;
        g->unk18 = 0;
        g->unk8 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        g->unk20 = extent >> 8;
        g->unk10 = extent >> 8;
        g->unk12 = 0;
        g->unkA = 0;
        g->unk22 = 0x35;
        g->unk1A = 0x35;
        p->unk0 = (p->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
        *ot = (*ot & 0xFF000000) | ((s32)p & 0xFFFFFF);
        p = (StructS0 *)((u8 *)p + 0x24);
    }
    return p;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80142D68(s32 arg0)
{
    Packet *p;
    s32 state;

    func_800A3938(0x78, 0x80);
    D_801468B8.unk0 = ((((D_801468B8.unk0 & ~0x78) | 8) & ~7 | 1) & 0xFFFF007F | 0x1000) & 0xFFFFFF;
    p = (Packet *)&D_801468B8;
    p->unk4 |= 1;
    if (arg0 < 2 || arg0 == 4)
    {
        p->unk4 = (p->unk4 & ~0x1FE) | 0x3E;
        state = 0x60;
    }
    else
    {
        p->unk4 = (p->unk4 & ~0x1FE) | 0x1E;
        state = 0x70;
    }
    ((u8 *)p)[2] = state;
    p->unk8 = func_80142E7C;
    func_800AA02C();
    D_80162370 = 0;
    D_8015A320 = 0;
    D_80162364 = 0;
    D_8015A310 = 0;
    D_80162350 = 0xFF;
    func_80143DE4();
    D_80162374 = 0;
    D_80162360 = arg0;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_80142E7C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 state = D_80162360;
    u8 *base;

    if (0)
    {
        func_80143520(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    switch (state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145ED8, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        base = (u8 *)&D_80145ED8 - 0x3C;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x56), 4, -arg2 + 0x80, -arg3 + 0x10, 2);
        break;
    case 1:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDA, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        base = (u8 *)&D_80145EDA - 0x3E;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x56), 4, -arg2 + 0x80, -arg3 + 0x10, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDC, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDE, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80145EDA, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        base = (u8 *)&D_80145EDA - 0x3E;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x5C), 4, -arg2 + 0x80, -arg3 + 0x10, 2);
        break;
    }

    if (D_80122988 & 0x220)
    {
        D_801468B8.unk0 &= ~7;
        func_800AA02C();
    }

    return prim;
}

typedef struct
{
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 unk25;
} StructFT4;

/**
 * @brief Draw one icon-highlight row: set the text scissor window and build a
 *        textured (POLY_FT4) quad for the icon at slot @p index, linked into
 *        the ordering table.
 * @param prim primitive cursor the FT4 quad is written into.
 * @param ot ordering-table head threaded through the OT-link update.
 * @param x quad left x (unk8/unk18).
 * @param y quad top y (unkA/unk12).
 * @param highlight x offset added for the quad's right edge (unk10/unk20).
 * @param icon icon id; 0x7F skips the whole draw, <2 / <0x4F / >=0x4F pick the
 *             scissor source (func_800A5638 vs the D_8015A314 table vs
 *             func_800A55E4 with D_8015A31C).
 * @param index slot index; drives the scissor x and the quad UV base (index*3).
 * @param row when 1 and icon<2 uses the func_800A5638 scissor path.
 * @return the advanced primitive cursor (prim + 0x28), or prim unchanged when
 *         icon == 0x7F.
 * @note WIP 85.14% (122/145 exact). Structure, control flow, the -0x40 frame,
 *       prim in s2 (via `base = index * 3` kept as a single post-branch LOCAL so
 *       local-alloc gives it s0 and the long-lived global prim keeps s2), and
 *       the OtTag setaddr link are all in place. Residue is packet-block
 *       scheduling in the final basic block: the target hoists the `highlight`
 *       (sp+0x50) load and the `prim->unk0` OT-read early to hide latency, which
 *       pushes the `uv = base * 0x10` shift later, whereas this compile emits the
 *       uv block right after the code bytes. sched_oracle reports the block as
 *       NON_LUID / regalloc-decided (not a clean sched1 emit-order fix), and the
 *       permuter (v2) only found scaffolding (a stray pointer split plus a junk
 *       `(double)` cast). Also 2 minor argdiff rows: the `row` compare temp
 *       lands a0 vs target v1, and the func_800A55E4 path loads D_8015A31C after
 *       &D_8015A330 (target loads it first; a statement-precompute was inert via
 *       copy-propagation). working/func_801430C8 kept for a future 100% pass.
 * @see decomp.me (85.14%) TODO
 */
s32 func_801430C8(s32 prim, s32 *ot, s32 x, s32 y, s32 highlight, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 base;
    u8 uv;
    u8 uv2;
    s16 tmp;
    s32 v;
    s32 *ctx;

    if (icon == 0x7F)
    {
        return prim;
    }

    rect.x = index * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    v = icon < 0x4F;
    if (row == 1)
    {
        if (icon < 2)
        {
            ctx = &D_8015A330;
            func_800A5638(ctx, icon);
            goto block_8;
        }
        v = icon < 0x4F;
    }
    if (v == 0)
    {
        ctx = &D_8015A330;
        func_800A55E4(ctx, D_8015A31C);
    block_8:
        func_80019A34(&rect, ctx);
        func_80019788(0);
    }
    else
    {
        func_80019A34(&rect, D_8015A314 + *(s32 *)(D_8015A314 + icon * 4 + 4));
    }

    base = index * 3;
    rect.x = (base * 4) + 0x140;
    rect.w = 0xC;
    rect.h = 0x30;
    rect.y = 0xD0;
    func_80019A34(&rect, D_8015A314 + *(s32 *)(D_8015A314 + icon * 4 + 4) + 0x20);

    ((StructFT4 *)prim)->unk4 = 0x808080;
    ((u8 *)prim)[3] = 9;
    ((u8 *)prim)[7] = 0x2C;
    ((StructFT4 *)prim)->unk18 = x;
    ((StructFT4 *)prim)->unk8 = x;
    ((StructFT4 *)prim)->unk12 = y;
    ((StructFT4 *)prim)->unkA = y;
    uv = base * 0x10;
    ((StructFT4 *)prim)->unk1C = uv;
    ((StructFT4 *)prim)->unkC = uv;
    uv2 = uv + 0x2F;
    ((StructFT4 *)prim)->unk24 = uv2;
    ((StructFT4 *)prim)->unk14 = uv2;
    ((StructFT4 *)prim)->unk15 = 0xD0;
    ((StructFT4 *)prim)->unkD = 0xD0;
    tmp = x + highlight;
    ((StructFT4 *)prim)->unk20 = tmp;
    ((StructFT4 *)prim)->unk10 = tmp;
    tmp = y + 0x2F;
    ((StructFT4 *)prim)->unk22 = tmp;
    ((StructFT4 *)prim)->unk1A = tmp;
    ((StructFT4 *)prim)->unk25 = 0xFF;
    ((StructFT4 *)prim)->unk1D = 0xFF;
    ((StructFT4 *)prim)->unkE = (index & 0x3F) | 0x7C80;
    ((StructFT4 *)prim)->unk16 = 5;
    ((OtTag *)prim)->addr = ((OtTag *)ot)->addr;
    ((OtTag *)ot)->addr = (u32)prim;

    return prim + 0x28;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_8014330C(void)
{
    D_801468B8.unk0 = D_801468B8.unk0 & ~7;
}

/**
 * @brief Load the CD icon-set resource into the 0x80180000 scratch buffer and
 *        register its three icon glyphs (codes 0x200/0x240/0x280) with ids
 *        0x1F4/0x1F5/0x1F6 via func_80086374, also pointing D_8015A314 (the
 *        icon UV table used by func_801430C8) at the resource's first offset
 *        field.
 * @note WIP 99.50% (45/50 exact, correct frame/insn count, no structural
 *       rows). The resource header at 0x80180000 stores four s32 offset
 *       fields at +4/+8/+0xC/+0x10; the target reads them via a SINGLE
 *       `lui $s0,%hi(D_80180008)` reused (as a raw 0x80180000 constant) for
 *       all four %lo(D_80180004/8/C/10) loads AND the "+ base" pointer
 *       conversions, with zero extra instructions. Every measured source
 *       shape that gives the four fields their own extern symbol (plain
 *       scalars: 72.76%, 4 separate luis; one struct/array anchored at the
 *       base: 82.68%, gcc instead MATERIALIZES a full address via an extra
 *       `addiu` + saved register since the fields are read across 3
 *       intervening calls; `&sym +/- k` anchor tricks: 75.82-88.64%, same
 *       materialization cost) costs extra insns and regresses the match.
 *       Raw pointer arithmetic on the literal `(u8*)0x80180000` (this file)
 *       reproduces the target's exact instruction count and frame with the
 *       four loads showing as plain register+offset instead of named
 *       relocations - that is the whole gap. Permuter (v2, ~29.5k
 *       iterations) converged on this same state and found nothing further.
 *       Same residual class as func_800A3728 in field_audio.c (98.97%, also
 *       left unmatched after 22 measured-inert variants) - gcc sharing one
 *       %hi() across several extern data symbols in a page is not reachable
 *       from any C shape tried so far. working/func_80143324 kept for a
 *       future pass.
 * @see decomp.me (99.50%) TODO
 */
void func_80143324(void)
{
    u8 *base;
    s16 buf[4];

    func_800141EC(0x5E4, (void *)0x80180000);
    func_80013F2C();

    base = (u8 *)0x80180000;

    D_8015A314 = *(s32 *)(base + 4) + (s32)base;

    buf[0] = 0x200;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0x1F4;
    func_80086374(buf, *(s32 *)(base + 8) + (s32)base, 1);

    buf[0] = 0x240;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0x1F5;
    func_80086374(buf, *(s32 *)(base + 0xC) + (s32)base, 1);

    buf[0] = 0x280;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0x1F6;
    func_80086374(buf, *(s32 *)(base + 0x10) + (s32)base, 1);
}

/**
 * @brief Build three OT-linked icon-highlight primitives (one 20-byte
 *        SPRT-shaped color record plus one 8-byte DR_MODE-style texture-page
 *        record per icon), stepping the color/row coordinates and
 *        texture-page code for each of the 3 slots.
 * @param p primitive buffer to write the 3 record pairs into.
 * @param ot ordering-table head threaded through each record's OT link.
 * @return the advanced primitive cursor after all 3 pairs (p + 0x54).
 * @note WIP 91.23% (63/73 exact, correct frame/insn count, no structural or
 *       scheduling rows left). Residual is a single swapped register pair:
 *       the OtTag bitfield's 0xFF000000 len-mask constant and the `code`
 *       (unk8) loop accumulator land in t3/t6 opposite of the target (mask
 *       should be t6, code should be t3). alloc_report confirms this is a
 *       genuine allocator priority tie broken the wrong way: mask
 *       pri=2142 (9 refs/126 live) vs code pri=2028 (7 refs/69 live), and
 *       explain_conflict confirms the two pseudos DO conflict, so it is not
 *       hand-swappable - gcc's own priority formula decides it. Measured
 *       inert: every declaration/init/increment-clause reordering of
 *       code/col/row (byte-identical output), an explicit s16 cast on the
 *       unk8 store, inlining vs naming `tpage`, and merging row/col into one
 *       accumulator via their constant 0x7B00 offset (both merge directions
 *       cost 12 exact rows - they are genuinely separate variables).
 *       Permuter (v2, 100k+ iterations across 3 re-seeds) never beat the
 *       seeded state. Shapes that ARE measured required: `col += 0x40,
 *       row += 0x40, code += 0x80, i++` in that exact order as the for-loop's
 *       increment clause (i last); `tpage` computed AFTER the first OtTag
 *       link and p-advance, not before. working/func_801433EC kept for a
 *       future pass.
 * @see decomp.me (91.23%) TODO
 */
StructS0 *func_801433EC(StructS0 *p, RenderCtx *ot)
{
    s32 i;
    s32 code;
    s32 col;
    s32 row;
    s32 tpage;

    code = 8;
    col = 0x200;
    row = 0x7D00;
    for (i = 0; i < 3; col += 0x40, row += 0x40, code += 0x80, i++)
    {
        p->unk4 = 0x808080;
        ((u8 *)p)[3] = 4;
        ((u8 *)p)[7] = 0x64;
        p->unk8 = code;
        p->unkA = 0;
        ((u8 *)p)[0xC] = 0;
        ((u8 *)p)[0xD] = 0;
        if (i == 2)
        {
            *(s16 *)((u8 *)p + 0x10) = 0x30;
        }
        else
        {
            *(s16 *)((u8 *)p + 0x10) = 0x80;
        }
        *(s16 *)((u8 *)p + 0x12) = 0xE0;
        p->unkE = row;
        ((OtTag *)p)->addr = ((OtTag *)ot)->addr;
        ((OtTag *)ot)->addr = (u32)p;
        p = (StructS0 *)((u8 *)p + 0x14);
        ((u8 *)p)[3] = 1;
        tpage = ((col & 0x3FF) >> 6) | 0xE1000080;
        p->unk4 = tpage;
        ((OtTag *)p)->addr = ((OtTag *)ot)->addr;
        ((OtTag *)ot)->addr = (u32)p;
        p = (StructS0 *)((u8 *)p + 8);
    }
    return p;
}
