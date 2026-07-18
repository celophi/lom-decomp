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
void func_8014E3C4(u32 content_id);

extern s32 g_menu_draw_early_out;
extern s32 D_801690B8[];
extern s32 D_80169414;
extern s32 D_80169558;

void* menu_slot_alloc(s32 arg0, void* rect);
s32 func_8014551C(s32 arg0);
s32 func_8014E8B8(void);
s32 func_8014E9A0(void);
s32 func_8014EA78(void);
s32* func_8014A3A4();
s32* func_8014BA58();
s32* func_8014BD48();
s32* func_8014BF68();
s32* func_8014C820();
s32* func_8014C8C8();

/**
 * @brief Scroll-list content callback for the item/technique list page.
 *
 * Circle (0x40) closes the page (SE 0x7F, state->unk0 = 3). Otherwise scans the
 * 11-word x 24-bit availability bitmask at g_pad_ctx + 0x34 to find the absolute
 * bit index of the sel_idx'th set bit. On confirm (0x220), if that entry's row
 * matches the active category ((D_801693FC->unk14 >> 10) & 0x3F), the per-char
 * slot byte at g_pad_ctx + slot * 0x250 + subtype + 0x609 is read: 0xFF or
 * 0x80-clear assigns the entry there (SE 0x7E); 0x80-set tries the
 * func_800A9060 / func_800A8F8C swap path, clearing all four g_menu_slots and
 * loading content 6 on failure. A wrong category plays SE 0x78. The draw pass
 * renders the list chrome via scroll_list_draw, then draws each set bit's glyph
 * string (func_800A88A0, color 1 on the active category's row) and finally
 * points g_menu_pending_overlay at the found entry's description.
 *
 * @param ot          Ordering-table pointer, forwarded to the glyph renderer.
 * @param arg1        Scroll-list state for this page (aliased into @c state).
 * @param arg2        Primitive buffer write cursor (aliased into @c prim).
 * @param view_origin Viewport anchor; glyph origins are (0x10 - x, rel - y).
 * @param active      Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @note Shapes required to match: @c flag_ptr and @c ctx must be SEPARATE
 *       variables (the target colors the 0x609 read chain v1 and the write
 *       chain a2; one shared local forces one color). The func_800A8F8C call
 *       and the 0x640 store must sit inside a do { } while (0) block - its
 *       block notes keep the off-chain addu out of the g_pad_ctx load-delay
 *       slot the scheduler must fill with it. @c off is assigned inside the
 *       store expression (operand-order idiom, as in func_8014B7DC). In the
 *       draw block, @c glyph is computed BEFORE the color compare, the
 *       D_801693FC deref stays inline in the compare (a named local outranks
 *       @c base and steals v1), and @c base accumulates via the two-step
 *       assignment.
 * @see decomp.me (100%)
 */
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
            u8* flag_ptr = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            u8* ctx;

            flag_ptr += g_menu_active_subtype;
            flag = *(flag_ptr + 0x609);
            if (flag != 0xFF)
            {
                if (flag & 0x80)
                {
                    v0 = func_800A9060();
                    if (v0 != 0)
                    {
                        s32 off;

                        do
                        {
                            func_800A8F8C(v0, (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) + 0x90));
                            *((u8*)g_pad_ctx + (off = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * 0x250)) + 0x640) = 0;
                        } while (0);
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
            ctx += g_menu_active_subtype;
            *(ctx + 0x609) = found % 24;
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
                        s32 offs;
                        u8* glyph;

                        color = 3;
                        base = g_menu_state_ptr;
                        base += *(s32*)(base + 0x20);
                        offs = *(u16*)((u8*)base + (j * 2) + (i * 0x30));
                        glyph = base + offs;
                        if (i == (s32)(((u32)(*(s32*)((u8*)D_801693FC + 0x14)) >> 0xA) & 0x3F))
                        {
                            color = 1;
                        }
                        prim = func_800A88A0(prim, ot, glyph, color, 0x10 - view_origin->x, rel - view_origin->y, 0);
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

/**
 * @brief Open the menu content window for the given content page id.
 *
 * Sets D_80169558 (pending item row) to 0xFF, then for ids 0-7 allocates a
 * MenuSlot window and installs its content callback:
 * - 0/1/2: 0xF0 x 0x60 window at (0x40, 0x60), callback func_8014A3A4; sets
 *   D_80169414 (active category) to the id and packs func_8014551C(id) into
 *   flags bits 24:16.
 * - 3/4/5: 0xE8 x 0x90/0x80/0x90 window at (0x40, 0x2C), callbacks
 *   func_8014BA58 / func_8014BD48 / func_8014BF68; sets D_80169414 = 3 and
 *   packs func_8014E8B8 / func_8014E9A0 / func_8014EA78 into flags bits 24:16.
 * - 6/7: 0x120 x 0x20/0x30 window at (0x10, 0x60), callbacks func_8014C820 /
 *   func_8014C8C8, flags bits 24:16 = 1; rebuilds D_801690B8 as a 1-entry
 *   circular nav list and sets g_menu_draw_early_out.
 * Ids >= 8 only reset D_80169558.
 *
 * @param content_id Content page id (0-7); out of range is a no-op beyond the
 *        D_80169558 reset.
 * @note Shapes required to match: the rect must be a single u16 rect[4] array
 *       (separate locals get their stores dead-eliminated since only the first
 *       address escapes), and the raw callee result must be held in @c v0 with
 *       the (& 0x1FF) << 16 done inside the flags assignment so gcc 2.7.2
 *       cross-jumping merges the case 1-5 tails at one label.
 * @see decomp.me (100%)
 */
void func_8014E3C4(u32 arg0)
{
    u16 rect[4];
    MenuSlot* slot;
    s32 v0;
    s32 j;
    s32 prev;
    s32 next;
    s32 more;
    s32 link;
    s32 word_self;
    s32 word_prev;

    D_80169558 = 0xFF;
    switch (arg0)
    {
    case 0:
        rect[0] = 0x40;
        rect[1] = 0x60;
        rect[2] = 0xF0;
        rect[3] = 0x60;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014A3A4;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014551C(0);
        D_80169414 = 0;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 1:
        rect[0] = 0x40;
        rect[1] = 0x60;
        rect[2] = 0xF0;
        rect[3] = 0x60;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014A3A4;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014551C(1);
        D_80169414 = 1;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 2:
        rect[0] = 0x40;
        rect[1] = 0x60;
        rect[2] = 0xF0;
        rect[3] = 0x60;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014A3A4;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014551C(2);
        D_80169414 = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 3:
        rect[0] = 0x40;
        rect[1] = 0x2C;
        rect[2] = 0xE8;
        rect[3] = 0x90;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014BA58;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014E8B8();
        D_80169414 = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 4:
        rect[0] = 0x40;
        rect[1] = 0x2C;
        rect[2] = 0xE8;
        rect[3] = 0x80;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014BD48;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014E9A0();
        D_80169414 = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 5:
        rect[0] = 0x40;
        rect[1] = 0x2C;
        rect[2] = 0xE8;
        rect[3] = 0x90;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014BF68;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014EA78();
        D_80169414 = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 6:
        rect[0] = 0x10;
        rect[1] = 0x60;
        rect[2] = 0x120;
        rect[3] = 0x20;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014C820;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = D_801690B8[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            D_801690B8[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            D_801690B8[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            D_801690B8[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;

    case 7:
        rect[0] = 0x10;
        rect[1] = 0x60;
        rect[2] = 0x120;
        rect[3] = 0x30;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014C8C8;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = D_801690B8[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            D_801690B8[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            D_801690B8[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            D_801690B8[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;
    }
}
