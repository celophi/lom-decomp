#include "common.h"

typedef struct
{
    u8 _pad15[0x15];
    u8 unk15;
    u8 _pad16[0x52 - 0x16];
    u16 unk52;
    u8 _pad54[0x5A - 0x54];
    u16 unk5A;
    u16 unk5C;
    u16 unk5E;
    u16 unk60;
    u16 unk62;
} Struct_8006429C;

typedef struct {
    u8* unk0;               // 0x00
    u32 unk4;               // 0x04
    u32 unk8;               // 0x08
    u32 unkC;               // 0x0C
    u32 unk10;              // 0x10
    u8  unk14;              // 0x14
    u8  unk15;              // 0x15
    u8  _pad16[2];          // 0x16
    u8  unk18;              // 0x18
    u8  unk19;              // 0x19
    u8  _pad1A;             // 0x1A
    u8  unk1B;              // 0x1B
    u8  unk1C;              // 0x1C
    u8  unk1D;              // 0x1D
    u8  _pad1E[0x49 - 0x1E];// 0x1E
    u8  unk49;              // 0x49
    u8  _pad4A[0x52 - 0x4A];// 0x4A
    u16 unk52;              // 0x52
    u16 unk54;              // 0x54
    u16 unk56;              // 0x56
    u16 unk58;              // 0x58
    u16 unk5A;              // 0x5A
    u16 unk5C;              // 0x5C
    u16 unk5E;              // 0x5E
    u16 unk60;              // 0x60
    u16 unk62;              // 0x62
    u16 unk64;              // 0x64
    u16 unk66;              // 0x66
    u16 unk68;              // 0x68
    u16 unk6A;              // 0x6A
    u16 unk6C;              // 0x6C
    u16 unk6E;              // 0x6E
    u16 unk70[16];          // 0x70
    u16 _pad90[4];          // 0x90
} Struct_801ED0CC;

typedef struct
{
    u8 _pad00[0x14];
    u32 unk14;
    u32 unk18;
    u16 unk1C;
    u16 unk1E;
    u16 unk20;
    u16 unk22;
    u16 unk24;
    u16 unk26;
    u16 unk28;
    u16 _pad2A[(0x34 - 0x2A) / 2];
    Struct_801ED0CC unk34[4];
} Struct_801ED000;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/** @brief libgpu free-size sprite primitive (20 bytes). */
typedef struct
{
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    s16 x0;
    s16 y0;
    u8 u0;
    u8 v0;
    u16 clut;
    s16 w;
    s16 h;
} SPRT;

void func_800640B4(Struct_8006429C* arg0);

extern s16 D_801ED028;
extern s32 D_801ED044;

/**
 * @brief Copy unk60/unk62 into unk5C/unk5E, zero unk15, store unk52 into unk5A,
 *        then call func_800640B4 with the same struct pointer.
 * @param arg0 Pointer to the target struct.
 * @see decomp.me (100%) TODO
 */
void func_8006429C(Struct_8006429C* arg0)
{
    u16 temp_v0 = (u16)arg0->unk60;
    u16 temp_v1 = (u16)arg0->unk62;
    u16 temp_a1 = arg0->unk52;

    arg0->unk15 = 0;
    arg0->unk5C = temp_v0;
    arg0->unk5E = temp_v1;
    arg0->unk5A = temp_a1;
    func_800640B4(arg0);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/OcIvj
 */
void func_800642D4(void)
{
    RECT rect;
    s32 var_a0;
    s32* var_v1;
    u32 hw_val;
    s32 mask;
    s32 limit;
    Struct_801ED000* hw_regs = (Struct_801ED000*)0x801ED000;

    cdrom_stream(0xB1, 0x801DE000);

    rect.x = 0x130;
    rect.y = 0x1FC;
    rect.w = 0x10;
    rect.h = 4;
    LoadImage(&rect, (u32*)0x801DE000);

    rect.x = 0x3C0;
    rect.y = 0x1E0;
    rect.w = 0x40;
    rect.h = 0x20;
    LoadImage(&rect, (u32*)0x801DE080);

    hw_val = 0xE100041F;
    var_a0 = 3;
    mask = -8;
    limit = -1;
    var_v1 = (s32*)0x801ED044;

    hw_regs->unk14 = hw_val;
    hw_regs->unk18 = hw_val;
    hw_regs->unk1C = 0x7F13;
    hw_regs->unk1E = 0x7FD3;
    hw_regs->unk20 = 0x7F53;
    hw_regs->unk22 = 0x7F93;
    hw_regs->unk24 = 0x7E93;
    hw_regs->unk26 = 0x7ED3;
    hw_regs->unk28 = 0;

    while (var_a0 != limit)
    {
        *var_v1 &= mask;
        var_a0 -= 1;
        var_v1 = (s32*)((u8*)var_v1 + 0x98);
    }

    DrawSync(0);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/FQwgy
 */
void func_800643E0(void)
{
    s32 var_a0;
    s32* var_v1;
    s32 mask;
    s32 limit;
    u32 temp;

    D_801ED028 = 0;
    var_a0 = 3;
    mask = -8;
    limit = -1;
    var_v1 = (s32*)0x801ED044;

    do
    {
        temp = *var_v1;
        var_a0 -= 1;
        temp &= mask;
        *var_v1 = temp;
        var_v1 = (s32*)((u8*)var_v1 + 0x98);
    } while (var_a0 != limit);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ej2U3
 */
void func_8006441C(void)
{
    if ((D_801ED044 & 7) == 4)
    {
        func_8006700C((void*)0x801ED034, 0);
    }
    do
    {

        Struct_801ED0CC* hw_regs = (Struct_801ED0CC*)0x801ED0CC;
        u32 unk10_val;

        hw_regs->unk6C = 0x100;
        hw_regs->unk64 = 0x100;
        hw_regs->unk6E = 0x60;
        hw_regs->unk66 = 0x60;
        hw_regs->unk56 = 0xFF0;
        hw_regs->unk52 = 0xFF0;
        hw_regs->unk5A = 0xFF0;
        hw_regs->unk58 = 0xC;
        hw_regs->unk68 = 0;
        hw_regs->unk5C = 0;
        hw_regs->unk60 = 0;
        hw_regs->unk6A = 0;
        hw_regs->unk5E = 0;
        hw_regs->unk62 = 0;
        hw_regs->unk15 = 0;
        hw_regs->unkC = 0;
        hw_regs->unk0 = 0;
        hw_regs->unk4 = 0;
        hw_regs->unk8 = 0;
        hw_regs->unk14 = 0;
        hw_regs->unk19 = 0;
        hw_regs->unk18 = 0;
        hw_regs->unk1B = 0;
        hw_regs->unk1C = 0;
        hw_regs->unk1D = 0;

        hw_regs->unk10 = ((((hw_regs->unk10 & ~7) | 6) & ~0xC0) | 0x800) & ~0x1000;
    } while (0);
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800644FC(SPRT* prim, u8* str, u16 mode)
{
    s32 count = 0;
    Struct_801ED0CC* st = (Struct_801ED0CC*)0x801ED0CC;
    u16* carry;
    s32 remaining;
    s32 start_x;
    s32 end_x;
    s32 tex_u;
    s32 tex_v;
    s32 col;
    s32 cols;

    st->unk49 = 1;
    st->unk1B = mode & 7;
    carry = (u16*)0x801ED13C;
    st->unk4 = 0;
    st->unk8 = 0;
    st->unk19 = 0;
    st->unk14 = 0;
    remaining = st->unk58;
    start_x = st->unk52 - st->unk5A;
    st->unk0 = str;
    while (--remaining != -1)
    {
        *carry = 0;
        carry += 1;
    }
    func_800632E0(st, 0);
    tex_u = start_x;
    st->unk5A = st->unk5A & 0xFFFC;
    end_x = st->unk52 - st->unk5A;
    tex_v = 0;
    while (tex_u >= 0x100)
    {
        tex_u -= 0x100;
        tex_v += 0xC;
    }
    remaining = ((end_x - start_x) + 3) >> 2;
    if (remaining != 0)
    {
        do
        {
            col = tex_u >> 2;
            prim->v0 = tex_v - 0x80;
            prim->u0 = tex_u;
            if ((col + remaining) >= 0x41)
            {
                cols = 0x40 - col;
                tex_u = 0;
                tex_v += 0xC;
                remaining -= cols;
            }
            else
            {
                cols = remaining;
                remaining = 0;
            }
            prim->w = cols * 4;
            prim->h = 0xC;
            if (mode >= 8)
            {
                prim->clut = 0x7F13;
            }
            else
            {
                prim->clut = 0x7FD3;
            }
            prim += 1;
            count += 1;
        } while (remaining != 0);
    }
    return count;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80064678(u16 index)
{
    Struct_801ED000* base = (Struct_801ED000*)0x801ED000;
    Struct_801ED0CC* st;
    Struct_801ED0CC* e;
    u32 flags;
    s32 n;
    s32 w;
    s32 x;
    s32 y;
    u16 rem;

    if ((D_801ED044 & 7) == 4)
    {
        func_8006700C((void*)0x801ED034, 0);
    }
    st = &base->unk34[index];
    if ((st->unk10 & 7) == 2)
    {
        func_8006700C(st, 0);
    }
    flags = st->unk10;
    if ((flags & 7) != 0)
    {
        st->unk10 = (flags & ~0x6000) | 0x2000;
        func_80066FBC(index);
        return;
    }
    func_80064A3C(st);
    if (st->unkC != 0)
    {
        if ((base->unk28 & 1) == 0)
        {
            st->unk10 &= ~8;
            base->unk28 |= 1;
        }
        else
        {
            st->unk10 |= 8;
            base->unk28 |= 2;
        }
    }
    x = 0;
    y = 0;
    n = index;
    e = &base->unk34[0];
    while (--n != -1)
    {
        if ((e->unk10 & 7) != 0)
        {
            x = e->unk64;
            y = e->unk66;
        }
        e += 1;
    }
    n = st->unk54;
    st->unk68 = x;
    st->unk5C = x;
    st->unk60 = x;
    st->unk6A = y;
    st->unk5E = y;
    st->unk62 = y;
    while (n > 0)
    {
        w = st->unk56;
        while (w > 0)
        {
            rem = 0x100 - x;
            if (w >= rem)
            {
                w -= rem;
                x = 0;
                y += st->unk58;
            }
            else
            {
                x += w;
                w = 0;
            }
        }
        n -= 0x10;
    }
    st->unk6C = x;
    st->unk64 = x;
    st->unk6E = y;
    st->unk66 = y;
}

/**
 * @see decomp.me (100%) TODO
 */
void func_80064878(u16 index)
{
    Struct_801ED000* base = (Struct_801ED000*)0x801ED000;
    Struct_801ED0CC* st;
    u32 flags;
    s32 h;
    s32 w;
    s32 x;
    s32 y;
    u16 rem;

    if ((D_801ED044 & 7) == 4)
    {
        func_8006700C((void*)0x801ED034, 0);
    }
    st = &base->unk34[index];
    if ((st->unk10 & 7) == 2)
    {
        func_8006700C(st, 0);
    }
    flags = st->unk10;
    if ((flags & 7) != 0)
    {
        st->unk10 = (flags & ~0x6000) | 0x4000;
        func_80066FBC(index);
        return;
    }
    func_80064A3C(st);
    if (st->unkC != 0)
    {
        if (index == 0)
        {
            st->unk10 &= ~8;
            base->unk28 |= 1;
        }
        else
        {
            st->unk10 |= 8;
            base->unk28 |= 2;
        }
    }
    if (index == 0)
    {
        x = 0;
        y = 0;
    }
    else
    {
        x = 0;
        y = 0x30;
    }
    h = st->unk54;
    st->unk68 = x;
    st->unk5C = x;
    st->unk60 = x;
    st->unk6A = y;
    st->unk5E = y;
    st->unk62 = y;
    while (h > 0)
    {
        w = st->unk56;
        while (w > 0)
        {
            rem = 0x100 - x;
            if (w >= rem)
            {
                w -= rem;
                x = 0;
                y += st->unk58;
            }
            else
            {
                x += w;
                w = 0;
            }
        }
        h -= 0x10;
    }
    st->unk6C = x;
    st->unk64 = x;
    st->unk6E = y;
    st->unk66 = y;
}