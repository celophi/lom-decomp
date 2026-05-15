#include "menu.h"

/* K&R-style declaration: original call site in menu_tick passes no explicit
 * argument and relies on register a0 (the caller's first parameter) being
 * live. Keep the empty parameter list to preserve that codegen exactly. */
void menu_build_grid();

void menu_init(void)
{
    volatile u8 padding;
    menu_clear_vram();
    menu_state_init();
    func_80141324();
    g_active_slot = -1;
    func_800AA02C();
    g_menu_unk_e8 = 0;
    menu_init_prim_rects();
    g_menu_frame = 0;
    g_script_cursor = 0;
    func_801423D8();
}

/**
 * decomp.me (99.64%) https://decomp.me/scratch/AGd9K
 */
void menu_init_prim_rects(void)
{
    s32 s0 = 0;

    /* Force base address into s3 early */
    u8* base = D_800FE778;

    s32 s2 = 0x20;
    s32 s1 = 0;
    s16 params[4];

    do
    {
        /* First call */
        params[0] = 0x110;
        params[1] = s0 + 0x1D8;
        params[2] = 0x10;
        params[3] = 1;
        func_80019A34(params, base + ((s1 >> 2) * 4));

        /* Second call */
        params[0] = (s0 == 2) ? 0x3E8 : 0x3F4;
        params[1] = (s0 == 0) ? 0x120 : 0x150;
        params[2] = 0xC;
        params[3] = 0x30;
        func_80019A34(params, base + ((s2 >> 2) * 4));

        s2 += 0x4A0;
        s0++;
        s1 += 0x4A0;
    } while (s0 < 3);
}

/**
 * decomp.me (97.74%) https://decomp.me/scratch/vmp4D
 */
void menu_tick(void* arg0)
{
    s32 v0;
    s32 v1;
    s32 s3;
    s32 var_s0;
    u16 temp_v1;
    s32 padding[2];
    menu_build_grid();
    v0 = g_menu_frame;
    v1 = D_800F22AC;
    s3 = *((s32*)(((u8*)arg0) + 0x4040));
    g_menu_frame = v0 + 1;
    D_800F22AC = v1 + 1;
    func_800A9E78(&g_menu_frame, &D_800F22AC);
    if (((*((u32*)(((u8*)D_8012271C) + 0x858))) & 0x80) && ((*((u8*)(((u8*)D_8012271C) + 0x840))) != 0))
    {
        D_80122988 |= D_801229FC;
    }
    v0 = D_80122988 & 0x5000;
    if (v0)
    {
        D_80122988 = v0;
    }
    v0 = D_80122988 & 0xF000;
    if (v0)
    {
        D_80122988 = v0;
    }
    v0 = D_80122988 & 0xF;
    if (v0)
    {
        D_80122988 = v0;
    }
    if (g_pad_input_latched != 0)
    {
        D_80122988 = 0;
    }
    g_pad_input_latched = D_80122988;
    if (D_801228C8 != 0)
    {
        s32 idx;
        u8* base = g_script_table;
        s32 off = D_801228C8;
        off = (off << 1) + off;
        off <<= 4;
        base += off;
        idx = g_script_cursor;
        base += idx * 2;
        D_80122988 = 0;
        temp_v1 = *((u16*)base);
        if (temp_v1 == (v0 = 0xFFFF))
        {
            if (D_801228C8 < 4)
            {
                var_s0 = 0;
                if (D_80122730 > 0)
                {
                    do
                    {
                        func_8014B69C(1);
                        var_s0++;
                    } while (var_s0 < D_80122730);
                }
                g_script_repeat_last = D_80122730;
            }
            D_801228C8 = 0;
        }
        else
        {
            D_80122988 = (s32)temp_v1;
            g_script_cursor = idx + 1;
        }
    }
    *((s32*)(((u8*)arg0) + 0x4040)) = s3;
    func_8014134C(arg0);
}

/**
 * decomp.me (75.58%) https://decomp.me/scratch/AW5Sa
 */
s32* menu_build_text_run(s32* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7)
{
    u8 sp10[0x90]; /* buffer – size matches target frame */
    s32 tmp, count, i;
    s32 *ptr0, *ptr1;
    s32 acc; /* accumulator for halfwords */
    u8 *base, *col;

    /* first call: fill buffer */
    func_800171CC(sp10, arg2, arg6);
    sp10[arg6] = 0;

    /* second call: get number of elements */
    count = func_800644FC(arg0, sp10, arg3);

    /* subtract halfword values according to arg7 */
    if (arg7 == 1)
    {
        /* signed halfword (lh) */
        ptr0 = arg0;
        for (i = 0; i < count; i++)
        {
            arg4 -= *(s16*)((char*)ptr0 + 0x10);
            ptr0 = (s32*)((char*)ptr0 + 0x14);
        }
    }
    else if (arg7 == 2)
    {
        /* unsigned halfword → (val << 16) >> 17 */
        ptr0 = arg0;
        for (i = 0; i < count; i++)
        {
            u16 val = *(u16*)((char*)ptr0 + 0x10);
            arg4 -= ((s16)val) >> 1; /* arithmetic right shift, matches sra */
            ptr0 = (s32*)((char*)ptr0 + 0x14);
        }
    }

    acc = 0;

    /* main loop – process each structure */
    if (count > 0)
    {
        base = (u8*)arg0;
        col = (u8*)arg1;
        tmp = arg4 + (arg5 << 16); /* constant used inside loop */

        do
        {
            /* SPRT primitive: pos, white tint, len=4, code=0x64 */
            *(s32*)(base + 0x8) = tmp + acc;
            *(u32*)(base + 0x4) = 0x808080U;
            setSprt(base);

            acc += *(s16*)(base + 0x10); /* accumulate halfword */

            /* link this SPRT into the OT chain headed at @c col */
            addPrim(col, base);

            /* advance to next structure (20 bytes) */
            base += 0x14;
            col += 0x14;
        } while (--count);
    }

    /* terminating DR_TPAGE primitive (tpage=0x1F) */
    setDrawTPage((DR_TPAGE*)base, 0, 0, 0x1F);
    addPrim(col, base);

    /* return pointer to offset 0x8 of the current structure */
    return (s32*)(base + 8);
}

/**
 * decomp.me (94.19%) https://decomp.me/scratch/ZtHxG
 */
void menu_build_grid(GpuWork* arg0)
{
    volatile u8 sp0;
    volatile u16 sp2;
    volatile u16 sp4;
    volatile u16 sp6;
    s32 var_t2;
    u8* var_a2;
    u8* var_t0;
    u8* var_t3;
    u8* temp_t1;
    GpuWork* t7 = arg0;
    GpuWork* t4 = t7;

    var_t3 = (u8*)D_80168AA8;
    var_t2 = 0;
    temp_t1 = t7->prim_tail;
    sp6 = 0xFF;
    sp4 = 0xFF;
    var_t0 = var_t3 + 8;
    sp2 = 0;
    *(u16*)&sp0 = 0;

    /* DR_AREA / texture-window primitive (GP0 0xE2) — leading delimiter */
    setTexWindow((DR_TWIN*)temp_t1, (RECT*)&sp0);
    addPrim(&t4->ot_head, temp_t1);

    temp_t1 += 0xC;
    var_a2 = temp_t1;

    do
    {
        /* SPRT primitive: white tint, len=4, code=0x64 */
        *(u32*)(var_a2 + 4) = 0x808080;
        *(u8*)(var_a2 + 3) = 4;
        *(u8*)(var_a2 + 7) = 0x64;
        *(u16*)(var_a2 + 0xC) = *(u16*)var_t3;
        *(u32*)(var_a2 + 8) = *(u32*)(var_t0 - 4);
        *(u32*)(var_a2 + 0x10) = *(u32*)var_t0;

        if (var_t2 >= 0x11)
        {
            *(u16*)(var_a2 + 0xE) = 0x7C81;
        }
        else
        {
            *(u16*)(var_a2 + 0xE) = 0x7C80;
        }

        var_t2++;
        var_t0 += 0xC;
        var_t3 += 0xC;

        addPrim(&t4->ot_head, var_a2);
        var_a2 += 0x14;
    } while (var_t2 < 0x1D);

    temp_t1 = var_a2;

    sp4 = 0xFF;
    sp6 = 0xFF;
    *(u16*)&sp0 = 0;
    sp2 = 0;

    /* DR_AREA / texture-window primitive (GP0 0xE2) — trailing delimiter */
    setTexWindow((DR_TWIN*)temp_t1, (RECT*)&sp0);
    addPrim(&t4->ot_head, temp_t1);

    temp_t1 += 0xC;
    /* DR_TPAGE primitive (tpage=5) */
    setDrawTPage((DR_TPAGE*)temp_t1, 0, 0, 5);
    addPrim(&t4->ot_head, temp_t1);

    t7->prim_tail = temp_t1 + 8;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/CKNIH
 */
void menu_clear_vram(void)
{
    RECT rect;

    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1F2;
    menu_upload_tim(&rect);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/A1YTp
 */
void menu_state_init(void)
{
    g_menu_state_ptr = &D_80151EBC;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/tG03R
 */
void menu_upload_tim(Rect16* arg0)
{
    u8* base = g_menu_tim;
    u8* s0 = base + 0xC;
    s32 s3 = *(s32*)(s0 + 8);
    Rect16 sp10;
    u16* p;
    int i;

    g_menu_tim_dy = *(s32*)(s0 + 0x14);

    /* First loop */
    sp10.x = arg0->w;
    sp10.y = arg0->h;
    sp10.w = 0x100;
    sp10.h = 1;

    p = (u16*)(base + 0x20);
    for (i = 0; i < 0x100; i++)
    {
        if (*p != 0)
            *p |= 0x8000;
        p++;
    }
    func_80019A34(&sp10, s0 + 0x14);

    /* Second call */
    sp10.x = arg0->x;
    sp10.y = arg0->y;
    {
        // Enforce specific instruction ordering: addiu a1, s3, 8 then addu a1, s0, a1
        u8* temp = s0 + (s3 + 8);
        sp10.w = *(u16*)(temp + 8);
        sp10.h = *(u16*)(temp + 10);
        func_80019A34(&sp10, temp + 0xC);
    }

    /* Third loop */
    sp10.x = arg0->w;
    sp10.y = arg0->h + 1;
    sp10.w = 0x100;
    sp10.h = 1;

    p = (u16*)(base + 0x822C);
    for (i = 0; i < 0x100; i++)
    {
        if (*p != 0)
            *p |= 0x8000;
        p++;
    }
    func_80019A34(&sp10, base + 0x822C);
}

/**
 * decomp.me (99.82%) https://decomp.me/scratch/Xng7v
 */
void* menu_slot_alloc(s32 arg0, void* arg1)
{
    s32 var_a2;
    MenuSlot* entry;
    u8* cur;
    u8* ptr;
    u32 temp;
    u32 mask;
    u16* src = (u16*)arg1;
    var_a2 = 0;
    ptr = (u8*)&g_menu_slots[0];
    cur = (u8*)&g_menu_slots[0];
    while (var_a2 < 4)
    {
        if ((*cur) == 0)
        {
            break;
        }
        var_a2++;
        cur += 0x24;
    }

    if (var_a2 < 0)
    {
        return (void*)(-1);
    }
    entry = (MenuSlot*)(ptr + (var_a2 * 0x24));
    *((u16*)(((u8*)entry) + 4)) = 0;
    temp = entry->flags;
    entry->active = 1;
    entry->unk1C = 0;
    entry->index = (u8)var_a2;
    entry->unk20 = 0;
    entry->unk2 = 0;
    mask = 0x1FFFFFF;
    temp = temp & mask;
    temp = temp | (((u32)arg0) << 25);
    entry->flags = temp;
    entry->x = src[0];
    entry->y = src[1];
    entry->w = src[2];
    entry->h = src[3];
    entry->unk10 = 0;
    entry->unk12 = 0;
    entry->unk14 = 0;
    entry->unk16 = 0;
    entry->unk18 = 0;
    entry->unk3 = 0;
    g_active_slot = var_a2;
    return (void*)entry;
}
