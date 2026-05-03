#include "title.h"

/**
 * decomp.me (100%) https://decomp.me/scratch/mEAXF
 */
s32 title_func_8004FC74(s32 arg0)
{
    s32 pad;
    S_801ED480* ptr = (S_801ED480*)0x801ED480;
    s32* base;
    u8* d8_base;
    u32 const_ff;
    s32 temp1, temp2;
    u8 d92;
    pad = arg0;

    func_80050244();
    func_80050300(0);
    func_80050394();
    base = (s32*)0x80100000; /* base address for D_80102640 */
    const_ff = 0xFF;
    d8_base = D_80042FD8;
    while (1)
    {
        func_800500CC(pad);
        ptr->unk0 = 0;
        ptr->unk2 = 0;
        ptr->unk4 = 0;
        ptr->unk8 = 0;
        ptr->unkC = 0;
        do
        {
            func_8004FDBC(pad);
        } while (base[0x990] == 0); /* 0x80102640 offset = 0x2640, 0x2640/4 = 0x990 */

        D_80042FB4 = VSync(-1);
        d92 = D_80102692;

        if (d92 == 0)
        {
            func_80052220(0);
            base[0x990] = 0; /* D_80102640 = 0 */
            if (func_8004FF48(pad) == 2)
            {
                GFX_Transition(0);
                continue;
            }
            return 3;
        }
        else if (d92 == 1)
        {
            return 7;
        }
        else if (d92 == const_ff)
        {
            func_80050374();
            return 8;
        }
        else
        {
            func_800227D0(0, 0x3C, 0);
            func_80052220(-1);
            D_8003EC9C = const_ff;
            temp1 = rand();
            temp2 = rand();
            *(s16*)(d8_base + 0xD4) = (s16)(temp1 | (temp2 << 0xF));
            return 0;
        }
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/bMLDn
 */
void func_8004FDBC(void* arg0)
{
    RECT rect;
    u_char* base = (u_char*)arg0;
    u_char* s0;
    u_char* s1;

    DrawSync(0);
    VSync(0);

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    ClearImage(&rect, 0, 0, 0);

    s0 = base;
    ClearOTagR((u_long*)(s0 + 0x40), 0x1000);
    ClearOTagR((u_long*)(s0 + 0xBD0C), 0x1000);
    PutDispEnv((DISPENV*)(s0 + 0x4040));
    func_800157DC();

    SetDispMask(1);

    for (;;)
    {
        s1 = s0 + 0x40;
        ClearOTagR((u_long*)s1, 0x1000);
        *(u_long**)(s0 + 0x80B8) = (u_long*)(s0 + 0x40B8);
        rand();
        VSync(1);
        func_8005041C(s0);
        func_80050734(s0);
        func_80050A50(s0);
        func_80050864();

        if (D_80102640 == 0)
        {
            DrawSync(0);
            func_800157B0(2);
            VSync(2);

            {
                void* tmp = base;
                if (s0 == base)
                {
                    tmp = s0 + 0xBCCC;
                }
                s0 = tmp;
            }

            PutDispEnv((DISPENV*)(s0 + 0x4040));
            PutDrawEnv((DRAWENV*)(s0 + 0x4054));
            DrawOTag((u_long*)(s1 + 0x3FFC));
            func_800157DC();
            cdrom_process_state();

            if (D_80102640 == 0)
                continue;
        }
        break;
    }

    func_800158E0();
    VSync(0);
    DrawSync(0);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/AKk7x
 */
s32 func_8004FF48(void* arg0)
{
    short rect[4];
    void* base;
    void* current;
    void* tmp;
    u_long* ot;

    base = arg0;

    func_80051234();
    GFX_Transition(0);
    func_80050718(0x100, 0x100, 0x100, 0x14);
    DrawSync(0);
    VSync(0);
    rect[0] = 0;
    rect[1] = 0;
    rect[2] = 0x140;
    rect[3] = 0x1D8;
    ClearImage((RECT*)rect, 0, 0, 0);
    current = base;
    ClearOTagR((u_long*)((char*)current + 0x40), 0x1000);
    ClearOTagR((u_long*)((char*)current + 0xBD0C), 0x1000);
    VSync(0);
    PutDispEnv((DISPENV*)((char*)current + 0x4040));
    func_800157DC();
    SetDispMask(1);
    do
    {
        ot = (u_long*)((char*)current + 0x40);
        ClearOTagR(ot, 0x1000);
        *(void**)((char*)current + 0x80B8) = (void*)((char*)current + 0x40B8);
        VSync(1);
        func_8005041C(current);
        func_800512A0(current);
        DrawSync(0);
        func_800157B0(2);
        VSync(2);
        tmp = base;
        if (current == base)
            tmp = (char*)current + 0xBCCC;
        current = tmp;
        PutDispEnv((DISPENV*)((char*)current + 0x4040));
        PutDrawEnv((DRAWENV*)((char*)current + 0x4054));
        DrawOTag((u_long*)((char*)ot + 0x3FFC));
        func_800157DC();
        cdrom_process_state();
    } while (D_80102640 == 0);
    func_800158E0();
    VSync(0);
    return D_80102640;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/evJur
 */
void func_800500CC(void* arg0)
{
    RECT rect;
    unsigned char* base = (unsigned char*)arg0;
    unsigned char* hw = (unsigned char*)0x801ED600; /* hardware registers */
    unsigned char* base2;                           /* base + 0x8000 */

    /* Clear hardware register bytes */
    hw[0x13F] = 0;
    hw[0x91] = 0;
    hw[0x140] = 0;
    hw[0x92] = 0;

    func_8002237C(0);
    SetGeomScreen(0x5DC);
    SetGeomOffset(0xA0, 0x78);

    /* Write shorts at offsets 0x40B0..0x40B6 */
    *(short*)(base + 0x40B0) = 0;
    *(short*)(base + 0x40B2) = 0;
    *(short*)(base + 0x40B4) = 0x140;
    *(short*)(base + 0x40B6) = 0xF0;

    /* Secondary base (s0 = arg0 + 0x8000) */
    base2 = base + 0x8000;
    *(short*)(base2 + 0x7D7C) = 0;
    *(short*)(base2 + 0x7D7E) = 0xE8;
    *(short*)(base2 + 0x7D80) = 0x140;
    *(short*)(base2 + 0x7D82) = 0xF0;

    DrawSync(0);
    VSync(0);

    /* Clear a 0x400×0x200 rectangle */
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);

    /* Set display environments */
    SetDefDispEnv((DISPENV*)(base + 0x4040), 0, 0, 0x140, 0xF0);
    SetDefDispEnv((DISPENV*)(base + 0xFD0C), 0, 0xE8, 0x140, 0xF0);

    /* Set drawing environments */
    SetDefDrawEnv((DRAWENV*)(base + 0x4054), 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv((DRAWENV*)(base + 0xFD20), 0, 8, 0x140, 0xE0);

    /* Clear two more bytes */
    *(base + 0xFD36) = 0; /* was ctx->unkFD36 */
    *(base + 0x406A) = 0; /* was ctx->unk406A */

    func_800503EC();
    func_80050718(0x100, 0x100, 0x100, 0x14);
    func_80050CAC();

    D_80102640 = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/6zUZp
 */
void func_80050244(void)
{
    u8* base;
    u32* off;

    if (((u32)(g_previousGameState - 2) >= 2U) && (g_previousGameState != 6) && (g_previousGameState != 7) &&
        (g_previousGameState != 5))
    {

        D_80102668 = 0x8013C000;
        cdrom_queue_read(0x15, (void*)0x80180000);
        cdrom_wait_queue_empty();

        base = (u8*)0x80180000;
        off = (u32*)0x80180004;

        bcopy(base + off[0], (u8*)D_80102668, (int)(off[1] - off[0]));

        func_80021FFC(D_80102668);
        func_80022AE8(base + off[1], 1);
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/mBQ6i
 */
void func_80050300(s32 arg0)
{
    u32* off;
    u8* base;

    cdrom_queue_read((arg0 + 0x17) & 0xFFFF, (void*)0x80180000);
    cdrom_wait_queue_empty();

    off = (u32*)0x80180004;
    base = (u8*)0x80180000;

    bcopy(base + off[0], (unsigned char*)&D_8003ECA0, (int)(off[1] - off[0]));
    func_80022AE8((s32)(base + off[1]), 1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/1cta3
 */
void func_80050374(void)
{
    func_80022068(0);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/xYPkq
 */
void func_80050394(void)
{
    func_80022040(&D_8003ECA0);
    FUN_8002279c(0, 0x7F);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ZuKeL
 */
void func_800503C4(s32 arg0, s32 arg1)
{
    func_8002216C(arg0, 0, arg1, 0x7F);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/m80gj
 */
void func_800503EC(void)
{
    D_80102658.unk0 = 0;
    D_80102658.unk4 = 0;
    D_80102658.unk8 = 0;

    D_80102648.unk0 = 0;
    D_80102648.unk4 = 0;
    D_80102648.unk8 = 0;
    D_80102648.unkC = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/fBro2
 */
void func_8005041C(void* arg0)
{
    ArgStruct* arg = (ArgStruct*)arg0;
    u32* var_t4 = (u32*)arg->unk80B8;
    u32* unk40_ptr = (u32*)(((u8*)arg) + 0x40);
    s32 temp_a2;
    s32 temp_a0;
    s32 temp_v1;
    s32 var_a1;
    if (D_80102648.unkC != 0)
    {
        temp_a2 = ((s32)(D_80102648.unk0 - D_80102658.unk0)) / ((s32)D_80102648.unkC);
        temp_a0 = ((s32)(D_80102648.unk4 - D_80102658.unk4)) / ((s32)D_80102648.unkC);
        temp_v1 = ((s32)(D_80102648.unk8 - D_80102658.unk8)) / ((s32)D_80102648.unkC);
        D_80102648.unkC = D_80102648.unkC - 1;
        D_80102658.unk0 = D_80102658.unk0 + temp_a2;
        D_80102658.unk4 = D_80102658.unk4 + temp_a0;
        D_80102658.unk8 = D_80102658.unk8 + temp_v1;
    }
    else
    {
        D_80102658.unk0 = D_80102648.unk0;
        D_80102658.unk4 = D_80102648.unk4;
        D_80102658.unk8 = D_80102648.unk8;
    }
    if (!(((D_80102658.unk0 == 0x100) && (D_80102658.unk4 == 0x100)) && (D_80102658.unk8 == 0x100)))
    {
        if (((s32)D_80102658.unk0) >= 0x101)
        {
            ((u8*)var_t4)[4] = ((u8)D_80102658.unk0) - 1;
            ((u8*)var_t4)[5] = ((u8)D_80102658.unk4) - 1;
            ((u8*)var_t4)[6] = ((u8)D_80102658.unk8) - 1;
        }
        else
        {
            if (D_80102658.unk0 == 0x100)
            {
                ((u8*)var_t4)[4] = 0;
            }
            else
            {
                ((u8*)var_t4)[4] = ~((u8)D_80102658.unk0);
            }
            if (D_80102658.unk4 == 0x100)
            {
                ((u8*)var_t4)[5] = 0;
            }
            else
            {
                ((u8*)var_t4)[5] = ~((u8)D_80102658.unk4);
            }
            if (D_80102658.unk8 == 0x100)
            {
                ((u8*)var_t4)[6] = 0;
            }
            else
            {
                ((u8*)var_t4)[6] = ~((u8)D_80102658.unk8);
            }
        }
        ((u8*)var_t4)[3] = 3;
        ((u8*)var_t4)[7] = 0x62;
        *((u16*)(((u8*)var_t4) + 12)) = 0x140;
        *((u16*)(((u8*)var_t4) + 10)) = 0;
        *((u16*)(((u8*)var_t4) + 8)) = 0;
        *((u16*)(((u8*)var_t4) + 14)) = 0xF0;
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        ;
        var_a1 = 0x25;
        var_t4 = (u32*)(((u8*)var_t4) + 0x10);
        if (((s32)D_80102658.unk0) < 0x101)
        {
            var_a1 = 0x45;
        }
        ((u8*)var_t4)[3] = 1;
        *((u32*)(((u8*)var_t4) + 4)) = (s32)(var_a1 | 0xE1000000);
        *var_t4 = ((*var_t4) & 0xFF000000) | ((*unk40_ptr) & 0xFFFFFF);
        *unk40_ptr = ((*unk40_ptr) & 0xFF000000) | (((u32)var_t4) & 0xFFFFFF);
        var_t4 = (u32*)(((u8*)var_t4) + 8);
    }
    arg->unk80B8 = var_t4;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/zxqdP
 */
void func_80050718(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    D_80102648.unk0 = arg0;
    D_80102648.unk4 = arg1;
    D_80102648.unk8 = arg2;
    D_80102648.unkC = arg3;
}

/**
 * decomp.me (94.41%) https://decomp.me/scratch/Lluk6
 */
void func_80050734(void* arg0)
{
    u8* base = (u8*)arg0;
    unsigned int new_var;
    u8* t3;
    u32* a2;
    s32 t0;
    s32 t1;
    s32 a3;
    u8* a1;
    s32 temp_v0;
    s32 temp_v1;

    a2 = *((u32**)(base + 0x80B8));

    t3 = base + 0x40;

    t0 = 0;

    t1 = 0x140;

    a3 = 0x40;

    while (t0 < 5)
    {
        temp_v1 = t1 & 0x3FF;

        t1 += 0x40;

        *((short*)((((u8*)a2) + 0x0E) + 0x12)) = (short)a3;
        *((short*)((((u8*)a2) + 0x0E) + 2)) = (short)a3;
        a3 += 0x40;
        temp_v0 = t0 << 6;
        t0++;
        *((short*)((((u8*)a2) + 0x0E) + 0x0A)) = (short)temp_v0;
        *((short*)((((u8*)a2) + 0x0E) - 6)) = (short)temp_v0;
        (((u8*)a2) + 0x0E)[-0x0B] = 9;
        (((u8*)a2) + 0x0E)[-7] = 0x2C;
        (((u8*)a2) + 0x0E)[-8] = 0x80;
        (((u8*)a2) + 0x0E)[-9] = 0x80;
        (((u8*)a2) + 0x0E)[-0x0A] = 0x80;
        *((short*)((((u8*)a2) + 0x0E) + 4)) = 0;
        *((short*)((((u8*)a2) + 0x0E) - 4)) = 0;
        *((short*)((((u8*)a2) + 0x0E) + 0x14)) = 0xE0;
        *((short*)((((u8*)a2) + 0x0E) + 0x0C)) = 0xE0;
        (((u8*)a2) + 0x0E)[0x0E] = 0;
        (((u8*)a2) + 0x0E)[-2] = 0;
        (((u8*)a2) + 0x0E)[0x16] = 0x40;
        (((u8*)a2) + 0x0E)[6] = 0x40;
        (((u8*)a2) + 0x0E)[7] = 8;
        (((u8*)a2) + 0x0E)[-1] = 8;
        (((u8*)a2) + 0x0E)[0x17] = 0xE8;
        (((u8*)a2) + 0x0E)[0x0F] = 0xE8;
        *((short*)((((u8*)a2) + 0x0E) + 8)) = (short)((temp_v1 >> 6) | 0x110);
        *((short*)((((u8*)a2) + 0x0E) + 0)) = 0x7840;
        new_var = ((u32)a2) & 0xFFFFFF;
        a1 += 0x28;
        *a2 = ((*a2) & 0xFF000000) | ((*((u32*)(t3 + 0x3FFC))) & 0xFFFFFF);
        *((u32*)(t3 + 0x3FFC)) = ((*((u32*)(t3 + 0x3FFC))) & 0xFF000000) | new_var;
        a2 = (u32*)(((u8*)a2) + 0x28);
    }
    *((u32**)(base + 0x80B8)) = a2;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/vmcmD
 */
void func_80050864(void)
{
    s32 op = 0x7C;
    func_80050FBC();
    if (D_801026A0 == 0)
    {
        D_80102640 = 1;
        D_80102692 = 0xFF;
        return;
    }
    D_801026A0 -= 1;
    if (D_8010269C & 0xA20)
    {
        func_800503C4(op, 0x80);
        D_80102640 = 1;
        return;
    }
    if (D_8010269C & 0x9000)
    {
        func_8005099C();
        func_800503C4(0x7D, 0x80);
    }
    else if (D_8010269C & 0x6100)
    {
        func_8005091C();
        func_800503C4(0x7D, 0x80);
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/6k8uV
 */
void func_8005091C(void)
{
    s32 a1;
    u8* var_v1;
    const s32 LIMIT = 16;
    a1 = D_80102692 + 1;
    if (a1 < LIMIT)
    {
        u8* base = D_80102670;  // forces lui/addiu first
        var_v1 = base + a1 * 2; // sll comes after
        while (1)
        {
            if (*var_v1 != 0)
            {
                break;
            }
            a1++;
            if (a1 < LIMIT)
            {
                var_v1 += 2;
                continue;
            }
            break;
        }
    }
    if (a1 == LIMIT)
    {
        D_80102690 = 0;
        D_80102692 = 0;
    }
    else
    {
        D_80102692 = (u8)a1;
        D_80102690++;
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/mmzjI
 */
void func_8005099C(s32 arg2)
{
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_v1_2;
    u8* var_a0;
    u8* var_v1;
    u8* base;
    u8 temp;
    var_a1 = D_80102692 - 1;
    if (var_a1 >= 0)
    {
        base = &D_80102670;
        var_v1 = base + (var_a1 * 2);
        while (var_a1 >= 0)
        {
            if ((*var_v1) != 0)
            {
                break;
            }
            var_a1--;
            var_v1 -= 2;
        }
    }
    if (var_a1 < 0)
    {
        var_v1_2 = 0;
        var_a1_2 = 0;
        do
        {
            var_a0 = &D_80102670;
            while (var_a1_2 < 0x10)
            {
                if ((*var_a0) != 0)
                {
                    var_v1_2++;
                    var_a2 = var_a1_2;
                }
                var_a1_2++;
                var_a0 += 2;
            };
        } while (0);

        D_80102690 = var_v1_2 - 1;
        D_80102692 = (u8)var_a2;
        return;
    }
    temp = D_80102690;
    D_80102692 = (u8)var_a1;
    D_80102690 = temp - 1;
    return;
}

/**
 * decomp.me (65.89%) https://decomp.me/scratch/K627m
 */
void func_80050A50(void* arg0)
{
    s32 temp_s4;
    s32 var_a1;
    s32 var_s0;
    s32 var_s2;
    s32 var_s3;
    s32 var_s6;
    u8* var_s1;
    s32 unk80B8;
    s32 var_v0;
    s32 result;
    temp_s4 = (s32)(((u8*)arg0) + 0x40);
    var_s6 = 0x88;
    var_s3 = 0xA0;
    var_s2 = 0;
    var_s0 = 0;
    unk80B8 = *((s32*)(((u8*)arg0) + 0x80B8));
    var_a1 = func_80050BD4(temp_s4, unk80B8, 0, 0x64, 0xC8, 0, 0x80, 1);
    var_s1 = &D_80102670;
    do
    {
        if ((*var_s1) != 0)
        {
            if (D_80102690 == var_s2)
            {
                var_v0 = 1;
            }
            else
            {
                var_v0 = 2;
            }
            var_a1 = func_80050BD4(temp_s4, var_a1, var_s0 + 1, var_s6, var_s3, 0, 0x80, var_v0) + 0x28;
            var_s2++;
            var_s3 += 0xC;
        }
        var_s0++;
        var_s1 += 2;
    } while (var_s0 < 0x10);
    result = func_80050BD4(temp_s4, var_a1, 7, 0x78, (6 * (2 * ((s32)D_80102690))) + 0x9D,
                           (s32)D_8007FD2C[(D_80102691 >> 2) & 3], 0x10, 0);
    *((s32*)(((u8*)arg0) + 0x80B8)) = result;
    D_80102691++;
}