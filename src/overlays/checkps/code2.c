#include "checkps.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/jqJzK
 */
void func_80051DD4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    s32 new_var;
    Sp20Data sp;
    s32 temp;
    u16 *new_var3;
    s32 new_var4;
    
    new_var4 = arg2;
    if (arg2 >= 0)
    {
    temp = arg2;
    }
    else
    {
    temp = new_var4 + 15;
    }
    temp >>= 4;
    new_var3 = &D_8005D030[temp];
    do { } while (0);
    new_var = temp << 4;
    sp.sp20 = *new_var3;
    
    new_var = arg2 - new_var;
    sp.sp22 = D_8005D030[new_var];
    sp.sp24 = 0;
    func_80051E58((void *) arg0, (s32 *) arg1, (u8 *) (&sp), arg3, arg4, 0, arg5);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/gVtK1
 */
void* func_80051E58(void *arg0, s32 *arg1, u8 *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6)
{
    u8* s = arg2;
    s32 count = 0;
    u32 old;
    u16 val;
    u8 *p;

    /* Count characters */
    if (*s >= 0x20) {
        p = s;
        do {
            val = *p;

            if (val >= 0x80)
            {
                p++;
            }

            p++;
            count++;

        } while (*p >= 0x20);
    }

    /* Alignment adjustment */
    switch (arg6)
    {
        case 1:
            arg3 -= 16 * count;
            break;

        case 2:
            arg3 -= 8 * count;
            break;

        case 0:
        default:
            break;
    }

    D_800894CC = arg3;
    D_800894C0 = arg3;
    D_800894C4 = arg4;

    /* Main loop */
    while (1)
    {
        unsigned long c = *s;

        if (c == 0x20) {
            s++;
            D_800894C0 += 0x10;
            continue;
        }

        if (c >= 0x80) {
            val = s[0];
            val = (val << 8) | s[1];
            s += 2;
        } else {
            if (c < 0x20) {
                break;
            }

            val = (u16)(*s - 0x7AE1);
            s++;
        }

        arg0 = func_80052004(arg0, arg1, val, arg5);
    }

    /* Final write */
    ((u8 *)arg0)[3] = 1;
    *((u32 *)((u8 *)arg0 + 4)) = 0xE100000F;

    old = *((u32 *)arg0);
    *((u32 *)arg0) = (old & 0xFF000000) | ((*arg1) & 0xFFFFFF);

    *arg1 = ((*arg1) & 0xFF000000) | (((u32)arg0) & 0xFFFFFF);

    return ((u8 *)arg0) + 8;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/6ygLn
 */
s32 func_80052004(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    u32 *ptr;
    u8 *font_data;
    unsigned int new_var3;
    int new_var4;
    s32 slot;
    int new_var;
    unsigned int new_var2;
    s32 new_var5;
    RECT rect;

    u8 *dest;
    int inc;
    int inc16;
    int outer;
    int middle;

    u16 mask;
    u8 font_byte;
    volatile u8 *vptr;
    u8 temp;

    new_var5 = arg2;
    slot = 0;
    new_var3 = new_var5 & 0xFFFF;
    ptr = D_800890C0;

    while (slot < 0x100) {
        if (new_var3 == ((u16)(*ptr))) {
            return func_80052218(arg0, arg1, slot);
        }
        slot++;
        ptr++;
    }

    font_data = func_8001687C(new_var5 & 0xFFFF);
    if (font_data == ((u8 *)-1)) {
        return arg0;
    }

    dest = D_800894C8;
    inc = arg3 + 1;
    inc16 = inc * 16;

    for (outer = 0; outer < 15; outer++) {
        new_var4 = inc16;

        for (middle = 0; middle < 2; middle++) {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++) {
                *dest = ((*font_data) & mask) ? (inc) : (0);

                mask >>= 1 & (new_var2 = 0xFFFFu);
                new_var = (*font_data) & mask;

                vptr = (volatile u8 *)dest;
                temp = *vptr;

                if (new_var) {
                    temp += new_var4;
                }

                *vptr = temp;

                mask >>= 1;
                dest++;
            }

            font_data++;
        }
    }

    slot = 0;
    while ((slot < 0x100) && (D_800890C0[slot] != 0)) {
        slot++;
    }

    if (slot == 0x100) {
        return arg0;
    }

    D_800890C0[slot] = new_var5 & (0xFFFF & 0xFFFFFFFFu);
    arg0 = func_80052218(arg0, arg1, slot);

    D_800894D0 = (slot % 16) * 4;
    D_800894D4 = slot & 0xF0;

    rect.w = 4;
    rect.h = 15;
    rect.x = D_800894D0 + 0x3C0;
    rect.y = D_800894D4;

    LoadImage(&rect, (u_long *)D_800894C8);
    DrawSync(0);

    D_800894C8 += 0x80;
    return arg0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/FyrJc
 */
void *func_80052218(void *arg0, s32 *arg1, s32 arg2)
{
    int new_var;
    s32 var_a0;
    SomeStruct *s = (SomeStruct *) arg0;
    s32 arg0_masked;
    s32 old_c0;
    s32 new_c0;
    s32 cond;
    
    D_800890C0[arg2] |= 0x10000;
    s->u.byte.unk3 = 3;
    s->unk7 = 0x7C;
    s->unk5 = 0x80;
    s->unk6 = 0x80;
    s->unk4 = 0x80;
    var_a0 = arg2;
    s->unk8 = (u16) D_800894C0;
    s->unkA = (u16) D_800894C4;
    
    if (arg2 < 0)
    {
        var_a0 = arg2 + 0xF;
    }
    
    s->unkC = (s8) ((arg2 - ((var_a0 >> 4) * 0x10)) * 0x10);
    s->unkD = (s8) (arg2 & 0xF0);
    s->unkE = 0x7FC0;
    s->u.unk0 = (s->u.unk0 & 0xFF000000) | ((*arg1) & 0xFFFFFF);
    arg0_masked = ((s32) arg0) & 0xFFFFFF;
    new_var = (*arg1) & 0xFF000000;
    arg0 = ((char *) arg0) + 0x14;
    
    old_c0 = D_800894C0;
    new_c0 = old_c0 + 0x10;
    cond = (old_c0 + 0x20) < 0x280;
    D_800894C0 = new_c0;
    *arg1 = new_var | arg0_masked;
    
    if (!cond)
    {
        D_800894C0 = D_800894CC;
        D_800894C4 += 0x10;
    }
    
    return arg0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/PpMnG
 */
void func_80052320(void) 
{
    s32 var_a0;
    u32 *var_v1;

    D_800894C8 = (s32)&D_800810C0;
    var_a0 = 0;
    var_v1 = (u32*)&D_800890C0;

    while (var_a0 < 0x100) {
        var_a0++;
        *var_v1 = (u32)(*(u16*)var_v1);
        var_v1++;
    }
}


/**
 * decomp.me link (100%) https://decomp.me/scratch/PuSGD
 */
void func_8005235C(void)
{
    int new_var2;
    s32 var_a0;
    s32 *var_v1;

    var_a0 = 0;
    new_var2 = 0x10000;
    var_v1 = &D_800890C0;

    do {
        if (!((*var_v1) & new_var2)) {
            *var_v1 = 0;
        }

        var_a0 += 1;
        var_v1 += 1;
    } while (var_a0 < 0x100);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/2UOve
 */
void func_8005239C(void) 
{
    s32 i;
    s32 *p;
    RECT rect;

    // First loop: zero 0x100 words from D_800890C0 to D_800890C0+0x3FC
    i = 0xFF;
    // Force two‑step address calculation: base address + 0x3FC
    p = &D_800890C0;
    p = (s32*)((u_long)p + 0x3FC);
    
    while (i >= 0) {
        *p = 0;
        p--;
        i--;
    }

    // Second loop: zero 0x8000 bytes from D_800810C0
    for (i = 0; i <= 0x7FFF; i++) {
        D_800810C0[i] = 0;
    }

    // Assign RECT fields in the exact order required by the target assembly:
    // y, w, x, then h (h goes into the delay slot after the x store)
    rect.y = 0x1FF;
    rect.w = 0x10;
    rect.x = 0;
    rect.h = 1;

    LoadImage(&rect, (u_long*)&D_8005D054);
}