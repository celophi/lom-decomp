#include "menu.h"

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    u8 unk0;        /* 0x00 - set to 3 to request a state change */
    u8 pad01;
    u8 unk2;        /* 0x02 - cleared when the page opens a sub-window */
    u8 pad03;
    u16 sel_idx;    /* 0x04 */
    u16 item_count; /* 0x06 */
    u16 base_x;     /* 0x08 */
    u16 base_y;     /* 0x0A */
    s16 viewport_w; /* 0x0C */
    s16 viewport_h; /* 0x0E */
    u16 scroll_x;   /* 0x10 */
    u16 scroll_y;   /* 0x12 */
    s16 target_x;   /* 0x14 */
    s16 target_y;   /* 0x16 */
    u8 lerp_steps;  /* 0x18 */
} ScrollListState;

extern s32 g_menu_pending_overlay;
extern s32 g_menu_char_slot;
extern s32 g_menu_active_subtype;
extern void* D_80168C70;
extern void* D_801690A8;
extern void* D_801693FC;

s32 scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active);
void func_800A8F8C();
void func_800A8FB4();
s32 func_800A9060();
s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
void func_8014F210(s32 sound_id, s32 volume);

s32 func_8014DEEC(s32* ot, ScrollListState* arg1, s32 arg2, Vec2s* view_origin, s32 active)
{
    ScrollListState* state = arg1;
    s32 prim = arg2;
    s32 found;
    s32 count;
    s32 i;
    s32 j;
    s32 mask;
    s32 word;
    s32* p;
    s32 rel;
    s32 scroll_y;
    u8 flag;
    s32 v0;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        func_8014F210(0x7F, 0x80);
        state->unk0 = 3;
        return prim;
    }

    found = -1;
    count = 0;
    i = count;
    p = (s32*)((u8*)g_pad_ctx + 0x34);
    do
    {
        j = 0;
        mask = 1;
        word = *p;
        do
        {
            if (word & mask)
            {
                if (state->sel_idx == (count >> 4))
                {
                    found = j + (i * 0x18);
                }
                count += 0x10;
            }
            j += 1;
            mask = mask << 1;
        } while (j < 0x18);
        i += 1;
        p += 1;
    } while (i < 0xB);

    if ((g_pad_input & 0x220) && (active != 0))
    {
        if ((found / 24) == (s32)(((u32)(*(s32*)((u8*)D_801693FC + 0x14)) >> 0xA) & 0x3F))
        {
            u8* ctx = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);

            flag = *(ctx + g_menu_active_subtype + 0x609);
            if (flag != 0xFF)
            {
                if (flag & 0x80)
                {
                    v0 = func_800A9060();
                    if (v0 != 0)
                    {
                        s32 off;

                        func_800A8F8C(v0, (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) + 0x90));
                        off = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * 0x250);
                        *((u8*)g_pad_ctx + off + 0x640) = 0;
                        func_800A8FB4(off);
                    }
                    else
                    {
                        s32 n = 3;
                        MenuSlot* pool;
                        s8* slot;
                        u8* b;

                        b = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 8);
                        pool = g_menu_slots;
                        slot = (s8*)pool + 0x6C;
                        D_801690A8 = b + *(u16*)(b + 0xAC);
                        do
                        {
                            *slot = 0;
                            n -= 1;
                            slot -= 0x24;
                        } while (n >= 0);
                        func_8014E3C4(6);
                        return prim;
                    }
                }
            }
            ctx = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            *(ctx + g_menu_active_subtype + 0x609) = found % 24;
            func_8014F210(0x7E, 0x80);
            state->unk0 = 3;
        }
        else
        {
            func_8014F210(0x78, 0x80);
        }
    }

    prim = scroll_list_draw(prim, ot, state, (u32*)&D_80168C70, view_origin, active);

    found = -1;
    count = 0;
    i = count;
    p = (s32*)((u8*)g_pad_ctx + 0x34);
    scroll_y = state->scroll_y;
    do
    {
        j = 0;
        mask = 1;
        do
        {
            if (*p & mask)
            {
                rel = count - scroll_y;
                if (rel >= -0xF)
                {
                    if (rel < state->viewport_h - 0x10)
                    {
                        s32 color;
                        u8* base;
                        u8* entry;
                        s32 offs;

                        color = 3;
                        base = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x20);
                        entry = (u8*)D_801693FC;
                        offs = *(u16*)((u8*)base + (j * 2) + (i * 0x30));
                        if (i == (s32)(((u32)(*(s32*)(entry + 0x14)) >> 0xA) & 0x3F))
                        {
                            color = 1;
                        }
                        prim = func_800A88A0(prim, ot, base + offs, color, 0x10 - view_origin->x, rel - view_origin->y, 0);
                    }
                }
                if (state->sel_idx == (count >> 4))
                {
                    found = j + (i * 0x18);
                }
                count += 0x10;
            }
            j += 1;
            mask = mask << 1;
        } while (j < 0x18);
        i += 1;
        p += 1;
    } while (i < 0xB);

    if (found != -1)
    {
        u8* base = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x1C);

        g_menu_pending_overlay = (s32)(base + *(u16*)(base + (found * 2)));
    }
    return prim;
}
