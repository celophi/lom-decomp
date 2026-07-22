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
extern u8 D_800F0BE0[];
extern u8 D_800F0BEC[];

s32 scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active);
void func_800A8F8C();
void func_800A8FB4();
s32 func_800A9060();
s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
void func_8014F210(s32 sound_id, s32 volume);
void* func_8014F060(void);
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

/**
 * @brief Count usable 4-bit entries in the pad-ctx table at +0x104 and rebuild
 *        the D_80168C70 circular nav list to that size.
 *
 * Scans 16 words (128 nibbles) at g_pad_ctx + 0x104; every nibble with value
 * >= 2 adds one entry. Clears D_80168C70[0], then packs each of the count
 * entries with the same three bit-fields as func_80145278 (bits 13:0 = i * 0x10,
 * bits 22:14 = circular prev, bits 30:23 = circular next).
 *
 * @return Number of entries counted (also the nav-list length).
 * @note Shapes required to match: the outer scan must index by @c i
 *       (*(u32*)((u8*)g_pad_ctx + i * 4 + 0x104)) so loop.c strength-reduces it
 *       to the a0 pointer while keeping i for the slti compare - a separate
 *       pointer walk lets check_dbra_loop reverse the loop into a countdown.
 *       The 0xF nibble mask must be a local so it stays in a register. The
 *       link loop must REUSE @c j (the nibble counter) as its induction
 *       variable and route @c prev through the @c word temp - both reuses merge
 *       pseudos so the allocator reproduces the target's a0/a1/a2 coloring.
 * @see decomp.me (100%)
 */
s32 func_8014E8B8(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u32 word;
    s32 mask;

    count = 0;
    i = count;
    mask = 0xF;
    do
    {
        word = *(u32*)((u8*)g_pad_ctx + (i * 4) + 0x104);
        j = 7;
        do
        {
            if ((word & mask) >= 2)
            {
                count += 1;
            }
            j -= 1;
            word = word >> 4;
        } while (j >= 0);
        i += 1;
    } while (i < 0x10);

    D_80168C70 = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
            temp_t0 = (j) + (s32*)&D_80168C70;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            word = var_a2;
            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((word & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            j += 1;
            temp_a3 = j < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = j;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
        } while (temp_a3 != 0);
    }
    return count;
}

/**
 * @brief Count active byte entries in the pad-ctx table at +0x25E0 and rebuild
 *        the D_80168C70 circular nav list to that size.
 *
 * Clears the byte at g_pad_ctx + 0x26DF, then scans the 0x100 bytes at
 * g_pad_ctx + 0x25E0 counting non-zero entries. Clears D_80168C70[0] and packs
 * each of the count entries with the same three bit-fields as func_80145278
 * (bits 13:0 = i * 0x10, bits 22:14 = circular prev, bits 30:23 = circular
 * next). Sibling of func_8014E8B8 (case 4 vs case 3 of func_8014E3C4).
 *
 * @return Number of entries counted (also the nav-list length).
 * @note Shapes required to match: unlike func_8014E8B8 this one keeps the
 *       func_80145278 next/copy pair (temp_a1 = j + 1; ...; j = temp_a1) and
 *       the byte scan is a plain pointer walk (gcc's countdown reversal IS the
 *       target here). The link loop must sit inside a do { } while (0) block -
 *       its block notes shorten the live ranges so j outranks the temp_t0
 *       address temp and takes a3, cascading t0/t5/t4/t3/t2 into place.
 * @see decomp.me (100%)
 */
s32 func_8014E9A0(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u8* p;

    count = 0;
    ((u8*)g_pad_ctx)[0x26DF] = 0;
    p = (u8*)g_pad_ctx + 0x25E0;
    i = 0xFF;
    do
    {
        if (*p != 0)
        {
            count += 1;
        }
        i -= 1;
        p += 1;
    } while (i >= 0);

    D_80168C70 = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
        do
        {
            temp_t0 = (j) + (s32*)&D_80168C70;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((var_a2 & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            temp_a1 = j + 1;
            temp_a3 = temp_a1 < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = temp_a1;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
            j = temp_a1;
        } while (temp_a3 != 0);
        } while (0);
    }
    return count;
}

/**
 * @brief Count entries with bit 0 set in the pad-ctx table at +0x2F0 and
 *        rebuild the D_80168C70 circular nav list to that size.
 *
 * Scans 0x40 records of stride 0xC at g_pad_ctx + 0x2F0, counting those whose
 * first byte has bit 0 set. Clears D_80168C70[0] and packs each of the count
 * entries with the same three bit-fields as func_80145278 (bits 13:0 = i * 0x10,
 * bits 22:14 = circular prev, bits 30:23 = circular next). Third sibling of
 * func_8014E8B8 / func_8014E9A0 (case 5 of func_8014E3C4).
 *
 * @return Number of entries counted (also the nav-list length).
 * @note Same shapes as func_8014E9A0: pointer-walk countdown scan, the
 *       func_80145278 next/copy pair, and the do { } while (0) wrapper around
 *       the link loop that recolors the j / temp_t0 allocation race.
 * @see decomp.me (100%)
 */
s32 func_8014EA78(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u8* p;

    count = 0;
    p = (u8*)g_pad_ctx + 0x2F0;
    i = 0x3F;
    do
    {
        if (*p & 1)
        {
            count += 1;
        }
        i -= 1;
        p += 0xC;
    } while (i >= 0);

    D_80168C70 = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
        do
        {
            temp_t0 = (j) + (s32*)&D_80168C70;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((var_a2 & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            temp_a1 = j + 1;
            temp_a3 = temp_a1 < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = temp_a1;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
            j = temp_a1;
        } while (temp_a3 != 0);
        } while (0);
    }
    return count;
}

typedef struct
{
    u32 unk0; /* Slot 0 data pointer. */
    u32 unk4; /* Slot 1 data pointer. */
    u32 unk8; /* Slot 2 data pointer. */
    u32 unkC; /* Slot 3 data pointer. */
} ItemSlotData;

typedef struct
{
    u8 slot0; /* Slot 0 occupied flag. */
    u8 slot1; /* Slot 1 occupied flag. */
    u8 slot2; /* Slot 2 occupied flag. */
    u8 slot3; /* Slot 3 occupied flag. */
} ItemSlotFlags;

extern ItemSlotData g_item_slot_data;
extern ItemSlotFlags g_item_slot_flags;
extern u8 D_800F0BF8[];

/**
 * @brief Try to commit the pending item into the active character's slot
 *        buffer at g_pad_ctx + char_slot * 0x250 + 0x640.
 *
 * func_8014ECA4 supplies the candidate 0x40-byte record (returns 0 on failure,
 * which aborts with return 0). If the slot buffer still equals the template at
 * D_800F0BF8 (byte-wise), the candidate is copied in, its first byte is
 * cleared, and g_item_slot_data.unk0 is zeroed. Otherwise the slot and the
 * candidate are exchanged through a stack buffer (three func_800A8F8C copies,
 * same pattern as func_8014DE5C) and g_item_slot_data.unk0 points at the
 * candidate. Either way g_item_slot_flags.slot0 is set.
 *
 * @return 1 if a record was committed/exchanged, 0 if func_8014ECA4 failed.
 * @note Shapes required to match: the equality scan must use the
 *       mismatch-goto shape (every break/flag rewrite loses rows). buf must be
 *       the FIRST local so its address is virtual-stack-vars + 0 and reload
 *       rematerializes it per call (any nonzero offset routes &buf through a
 *       pseudo that cse merges across calls into a saved register). The
 *       `if (0)` six-arg call is required for that: gcc sizes the outgoing arg
 *       area (0x18) at expand time and jump1 deletes the call afterwards,
 *       which is how the original (likely a compiled-out debug call) got
 *       buf to sp+0x18. Residual 3 rows: the equal arm's a1 copy and
 *       %hi(g_pad_ctx) are scheduled earlier in the target (see
 *       working/func_8014EB4C/status.md).
 * @see decomp.me (96.92%)
 */
s32 func_8014EB4C(void)
{
    u8 buf[0x40];
    u8* entry;
    u8* t;
    u8* p;
    u8* slot_buf;
    u32 i;
    s32 diff;

    if (0)
    {
        func_800A8F8C(0, 0, 0, 0, 0, 0);
    }
    entry = (u8*)func_8014ECA4();
    if (entry != 0)
    {
        t = D_800F0BF8;
        i = 0;
        p = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640;
        do
        {
            i += 1;
            if (*t != *p)
            {
                diff = 1;
                goto done;
            }
            t += 1;
            p += 1;
        } while (i < 0x40);
        diff = 0;
done:
        if (diff == 0)
        {
            func_800A8F8C((u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640, entry);
            *entry = 0;
            g_item_slot_data.unk0 = 0;
        }
        else
        {
            slot_buf = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640;
            func_800A8F8C(buf, slot_buf);
            func_800A8F8C(slot_buf, entry);
            func_800A8F8C(entry, buf);
            g_item_slot_data.unk0 = (u32)entry;
        }
        g_item_slot_flags.slot0 = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Select the highest-valued eligible item record from the pad-context item table.
 *
 * Builds an exclusion mask from the other three active character-slot records, then
 * scans the 100 records at g_pad_ctx + 0xCE0. Records using the alternate lookup table
 * or an excluded category are skipped; the eligible record with the greatest halfword
 * value at offset 0x24 is returned.
 *
 * @return Pointer to the selected 0x40-byte record, or NULL if no eligible record exists.
 * @note The remaining mismatch is a pure a1/a2/a3 allocation rotation; instruction
 *       count and control-flow structure match the target. Continue from
 *       working/func_8014ECA4/.
 * @see decomp.me (98.84%)
 */
void* func_8014ECA4(void)
{
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 temp_v1_3;
    s32 var_t1;
    u32 temp_v1;
    u32 temp_v1_2;
    u8* var_v0;
    u8* temp_v0;
    u8* var_a2;
    u8* var_a3;
    u8* var_t0;

    var_t1 = 0;
    temp_v0 = (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0);
    var_t0 = temp_v0 + 0x50;
    if (*(u8*)(temp_v0 + 0x50) != 0)
    {
        var_t1 = *(u16*)(var_t0 + 0x24);
    }
    var_a3 = (u8*)g_pad_ctx + 0xCE0;
    var_t0 = 0;
    var_a1 = 0;
    var_a2 = (u8*)D_801693FC;
    var_a0 = 0;
    do
    {
        if ((var_a0 != 0) && (*var_a2 != 0))
        {
            temp_v1_2 = *(u32*)(var_a2 + 0x14);
            temp_v1 = temp_v1_2;
            if (temp_v1 & 0x300)
            {
                var_v0 = ((temp_v1 >> 0xA) & 0x3F) + D_800F0BEC;
            }
            else
            {
                var_v0 = ((temp_v1 >> 0xA) & 0x3F) + D_800F0BE0;
            }
            var_a1 |= *var_v0;
        }
        var_a0 += 1;
        var_a2 += 0x40;
    } while (var_a0 < 4);
    var_a0_2 = 0;
    do
    {
        if (*var_a3 != 0)
        {
            temp_v1_2 = *(u32*)(var_a3 + 0x14);
            if (!(temp_v1_2 & 0x300) && !(var_a1 & D_800F0BE0[(temp_v1_2 >> 0xA) & 0x3F]))
            {
                temp_v1_3 = *(u16*)(var_a3 + 0x24);
                if (var_t1 < temp_v1_3)
                {
                    var_t0 = var_a3;
                    var_t1 = temp_v1_3;
                }
            }
        }
        var_a0_2 += 1;
        var_a3 += 0x40;
    } while (var_a0_2 < 0x64);
    return var_t0;
}

/** @brief Slot-data pointer table indexed by menu subtype; @ref g_item_slot_data is its entry 7. */
extern u32 D_8016951C[];
/** @brief Slot-occupied flag table indexed by menu subtype; @ref g_item_slot_flags is its entry 7. */
extern u8 D_80168C15[];

/**
 * @brief Commit the pending item into the active character's slot buffer for the
 *        CURRENT menu subtype (@ref g_menu_active_subtype).
 *
 * The subtype-indexed sibling of @ref func_8014EB4C, which handles subtype 7
 * (slot 0) only. func_8014F060 supplies the candidate 0x40-byte record; a NULL
 * return aborts with 0. Any of comparison slots 1..3 that still points at this
 * subtype's slot buffer is re-pointed at it first.
 *
 * If the slot buffer is empty (its first byte is 0) the candidate is copied in,
 * its own first byte cleared, and the subtype's slot-data entry zeroed.
 * Otherwise the slot and the candidate are exchanged through a stack buffer
 * (three func_800A8F8C copies, same pattern as func_8014EB4C / func_8014DE5C)
 * and the subtype's slot-data entry points at the candidate. Either way the
 * subtype's slot-occupied flag is set.
 *
 * @return 1 if a record was committed or exchanged, 0 if func_8014F060 failed.
 *
 * @note D_8016951C and D_80168C15 are the bases of the subtype-indexed tables
 *       that @ref g_item_slot_data (0x80169538 = D_8016951C + 7*4) and
 *       @ref g_item_slot_flags (0x80168C1C = D_80168C15 + 7) sit inside. The
 *       relocation must name these bases, not `&g_item_slot_data - 0x1C`:
 *       the addresses are equal but the emitted symbol is not.
 *
 * @note Shapes required to match:
 *       - The two found-store blocks are OUT OF LINE. gcc 2.7.2 has no
 *         basic-block reorder pass, so the emitted order is the source order:
 *         arm A's store sits AFTER its common code and jumps backwards into it,
 *         and arm B's store sits BEFORE its scan loop, reached only by the
 *         loop's branch. The leading `goto scan_b;` is deleted by jump.c's
 *         jump-to-jump redirection but is what places found_b ahead of the
 *         loop. Writing either store inline with `break` costs ~27%: the store's
 *         address expression becomes loop-invariant, loop.c hoists it to the
 *         preheader, and CSE then shares it with the following call argument
 *         instead of recomputing it.
 *       - `slots[i]` indexing (not a `p = &g_item_slot_data.unk4` pointer walk)
 *         is what makes loop.c strength-reduce to the target's two-step
 *         `addiu v0, v0, %lo(g_item_slot_data)` + `addiu a1, v0, 0x4`.
 *       - `slots` must be assigned separately inside each arm; hoisting it
 *         above the `if` costs a fifth saved register (frame 0x68 -> 0x70).
 *       - `pad` aliases g_pad_ctx so its %hi becomes a long-lived allocno and
 *         its `lui` is emitted early, and `off` is assigned before it so the
 *         subtype `lui` comes first.
 *       - `ret` carries the 1 into the shared tail, producing the target's
 *         `li v0, 1` + `addu v1, v0, zero` pair. It only works assigned at the
 *         very end of the if/else; hoisting it anywhere earlier adds two insns.
 *       - `pad - (-(offset))` rather than `pad + offset`: gcc canonicalizes the
 *         operands of a commutative PLUS itself, so the sum coalesces into the
 *         char-slot chain's register. Routing it through MINUS preserves the
 *         operand order and gives the target's `addu a0, a0, v0`. Every
 *         plain-`+` spelling tried tops out at 99.87%.
 *       - The `if (0)` six-arg call sizes the outgoing-argument area to 0x18 so
 *         `buf` lands at sp+0x18; see func_8014EB4C. `&buf` must never be
 *         cached in a named local - it has to rematerialize at each call.
 * @see decomp.me (100%)
 */
s32 func_8014EDEC(void)
{
    u8 buf[0x40];
    u8* entry;
    u8* slot_buf;
    u32* slots;
    u8* pad;
    s32 ret;
    s32 off;
    s32 i;

    if (0)
    {
        func_800A8F8C(0, 0, 0, 0, 0, 0);
    }
    entry = (u8*)func_8014F060();
    if (entry != 0)
    {
        i = 1;
        off = g_menu_active_subtype - 7;
        pad = (u8*)g_pad_ctx;
        if (*(pad - (-((off << 6) + (g_menu_char_slot * 0x250))) + 0x640) == 0)
        {
            slots = &g_item_slot_data.unk0;
            do
            {
                if ((u32)entry == slots[i])
                {
                    goto found_a;
                }
                i += 1;
            } while (i < 4);
done_a:
            func_800A8F8C((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170), entry);
            *entry = 0;
            D_8016951C[g_menu_active_subtype] = 0;
            goto tail;
found_a:
            slots[i] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
            goto done_a;
        }
        else
        {
            goto scan_b;
found_b:
            slots[i] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
            goto done_b;
scan_b:
            slots = &g_item_slot_data.unk0;
            do
            {
                if ((u32)entry == slots[i])
                {
                    goto found_b;
                }
                i += 1;
            } while (i < 4);
done_b:
            slot_buf = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + (g_menu_active_subtype << 6) + 0x480;
            func_800A8F8C(buf, slot_buf);
            func_800A8F8C(slot_buf, entry);
            func_800A8F8C(entry, buf);
            D_8016951C[g_menu_active_subtype] = (u32)entry;
        }
        ret = 1;
tail:
        D_80168C15[g_menu_active_subtype] = ret;
        return 1;
    }
    return 0;
}

/**
 * @brief Pick the highest-valued eligible item record for the CURRENT menu subtype.
 *
 * The subtype-generic sibling of @ref func_8014ECA4 (which handles subtype 7 only).
 * The active character's slot buffer for @ref g_menu_active_subtype supplies the
 * score to beat: the sum of its four halfwords at 0x24/0x26/0x28/0x2A, or 0 when
 * the slot is empty. An exclusion mask is then built from the other three
 * comparison slots at D_801693FC, and the 100 records at g_pad_ctx + 0xCE0 are
 * scanned for the best record that is not excluded.
 *
 * @return Pointer to the winning 0x40-byte record, or NULL if none qualifies.
 *
 * @note Shapes required to match:
 *       - The opening test is a real `if/else` (`if (empty) { total = 0; } else
 *         { total = sum; }`), NOT `total = 0;` followed by a bare `if`. The
 *         target re-reads g_pad_ctx AND g_menu_active_subtype after the test
 *         (0x94-0xA8). With the bare `if`, gcc 2.7.2's CSE takes the AROUND
 *         path (`-fcse-skip-blocks`, on at -O2): cse_end_of_basic_block follows
 *         the branch around the if-body straight into the join block with its
 *         value table intact, so both globals are still live in registers there
 *         and the reloads fold away. The else arm puts a BARRIER in the way,
 *         CSE stops at the join, and both globals are re-read. Worth 9.36%.
 *       - `slot_idx` is deliberately reused after loop A as the loop-B mask
 *         carrier, and `mask` is reused inside loop B as the staging temp for
 *         `best_total`. gcc 2.7 has no live-range splitting, so one C variable
 *         is one hard register for the whole function; these two reuses are
 *         what give the target's a3 and a2 their second lives. A fresh variable
 *         in either place is coalesced away and costs the move.
 *       - `rec_flag` and `rec` walk the same records but must be ONE variable
 *         each in the roles shown: `rec_flag` for the occupancy byte, `rec` for
 *         the fields. Splitting `rec_flag` into an extra copy raises its
 *         allocation priority (pri = floor_log2(refs)*refs/live_len) above the
 *         mask carrier's and swaps a3/t0.
 * @note Measured NON-factors (probed at the 100% base, all still 100%): the
 *       `- (-(offset))` minus routing that @ref func_8014EDEC needs is NOT
 *       needed here, and neither is splitting `char_base` out of the `best`
 *       expression. Plain `+` is used because it is the readable form and it
 *       matches; do not reintroduce the minus spelling on the assumption that
 *       a sibling's requirement transfers.
 * @see decomp.me (100%)
 */
void* func_8014F060(void)
{
    s32 slot_idx;
    s32 rec_idx;
    s32 mask;
    s32 total;
    s32 best_total;
    u32 slot_flags;
    u32 rec_flags;
    u8* category;
    u8* char_base;
    u8* slot;
    u8* rec_flag;
    u8* rec;
    u8* best;

    char_base = (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0);
    best = char_base + ((g_menu_active_subtype << 6) - 0x170);
    if (*best == 0)
    {
        best_total = 0;
    }
    else
    {
        best_total = *(u16*)(best + 0x24) + *(u16*)(best + 0x26) + *(u16*)(best + 0x28) + *(u16*)(best + 0x2A);
    }
    best = 0;
    mask = 0;
    slot_idx = 0;
    slot = (u8*)D_801693FC;
    rec_flag = (u8*)g_pad_ctx + 0xCE0;
    do
    {
        if ((slot_idx != (g_menu_active_subtype - 7)) && (*slot != 0))
        {
            slot_flags = *(u32*)(slot + 0x14);
            if (slot_flags & 0x300)
            {
                category = ((slot_flags >> 0xA) & 0x3F) + D_800F0BEC;
            }
            else
            {
                category = ((slot_flags >> 0xA) & 0x3F) + D_800F0BE0;
            }
            mask |= *category;
        }
        slot_idx += 1;
        slot += 0x40;
    } while (slot_idx < 4);
    slot_idx = mask;
    rec_idx = 0;
    rec = rec_flag;
    do
    {
        if (*rec_flag != 0)
        {
            rec_flags = *(u32*)(rec + 0x14);
            if (((rec_flags & 0x300) == 0x100) && !(slot_idx & D_800F0BEC[(rec_flags >> 0xA) & 0x3F]))
            {
                total = *(u16*)(rec + 0x24) + *(u16*)(rec + 0x26) + *(u16*)(rec + 0x28) + *(u16*)(rec + 0x2A);
                if (best_total < total)
                {
                    best = rec;
                    mask = total;
                    best_total = mask;
                }
            }
        }
        rec_idx += 1;
        rec += 0x40;
        rec_flag += 0x40;
    } while (rec_idx < 0x64);
    return best;
}

/**
 * @brief Play a menu sound effect, unless a menu script is currently driving input.
 * @param sound_id Sound effect ID (see the MENU_SE_* constants in menu.c).
 * @param volume Playback volume (menu callers always pass 0x80).
 * @note Suppressed while @c g_active_script is non-zero so scripted/demo input
 *       replay does not retrigger UI blips.
 * @note func_800A3938 is left implicitly declared here, matching the rest of this
 *       translation unit. Measured non-factor: adding an explicit prototype also
 *       gives 100%.
 * @see decomp.me (100%)
 */
void func_8014F210(s32 sound_id, s32 volume)
{
    if (g_active_script == 0)
    {
        func_800A3938(sound_id, volume);
    }
}

/**
 * @brief Count the in-use entries in the 100-slot record table at g_pad_ctx+0xCE0.
 * @return Index of the first empty (zero first byte) record, i.e. the number of
 *         occupied records; 0x64 if the table is full.
 * @note Records are 0x40 bytes apart; the same table is walked by func_8014F060.
 * @note The `for` loop with a `break` on the empty slot is required to match: the
 *       equivalent `do { if (!*rec) break; ... } while (count < 0x64)` scores
 *       39.43% (gcc rotates and peels the loop) and the `while (*rec != 0)` form
 *       with the bound check inside scores 32.64%. A literal goto/label version of
 *       this same shape also reaches 100%.
 * @see decomp.me (100%)
 */
s32 func_8014F23C(void)
{
    s32 count;
    u8* rec;

    rec = (u8*)g_pad_ctx + 0xCE0;
    for (count = 0; count < 0x64; count++)
    {
        if (*rec == 0)
        {
            break;
        }
        rec += 0x40;
    }
    return count;
}

/**
 * @brief Draw a number through func_800A8A78, clamped to a maximum of 99.
 * @param prim Primitive buffer write cursor.
 * @param cursor Glyph write cursor, forwarded unchanged.
 * @param value Number to draw; anything >= 100 is drawn as 99.
 * @param arg3 Forwarded unchanged. TODO: meaning unknown (call sites pass 1).
 * @param origin Viewport anchor, forwarded unchanged.
 * @param color Palette index, forwarded unchanged.
 * @note Every other arg is a pure pass-through, which is why the asm only ever
 *       touches a2 and re-stages the two stack args.
 * @note Measured non-factors (all 100%): declaring this s32 and returning the
 *       callee's result, writing the clamp as @c value>0x63, and clamping into a
 *       separate local instead of the parameter.
 * @see decomp.me (100%)
 */
void func_8014F274(s32 prim, s32 cursor, s32 value, s32 arg3, Vec2s* origin, s32 color)
{
    if (value >= 0x64)
    {
        value = 0x63;
    }
    func_800A8A78(prim, cursor, value, arg3, origin, color);
}

extern s32 D_8016B760;
extern s32 D_8016B764;
extern s32 D_8016B768;
extern s32 D_8016B76C;
extern s32 D_8016B770;
extern s32 D_8016B774;
extern s32 D_8016B778;
extern s32 D_8016B77C;

/**
 * @brief Open and enable the eight memory-card events used by the save/load menu.
 *
 * Opens the four standard card event specs (end-of-IO, error, timeout, new
 * device) against both the BIOS card driver (@c SwCARD) and the card hardware
 * (@c HwCARD), stashes the eight descriptors in D_8016B760..D_8016B77C, then
 * enables all eight. The whole sequence runs inside a critical section.
 *
 * @note All eight events use @c EvMdNOINTR with a NULL handler, i.e. they are
 *       polled via TestEvent rather than delivering callbacks.
 * @note The eight descriptors are separate globals, not an array: the target
 *       emits a distinct @c lui per symbol and pins seven of them in saved
 *       registers, which an array base would not do.
 * @note kernel.h and libapi.h are already in scope through menu.h -> main.h ->
 *       libapi.h, so the named constants need no extra include. Measured
 *       non-factor: the raw hex form (0xF4000001, 4, 0x2000, 0) is also 100%.
 * @see decomp.me (100%)
 */
void func_8014F2B0(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    D_8016B760 = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    D_8016B764 = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    D_8016B768 = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    D_8016B76C = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    D_8016B770 = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    D_8016B774 = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    D_8016B778 = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    D_8016B77C = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    EnableEvent(D_8016B760);
    EnableEvent(D_8016B764);
    EnableEvent(D_8016B768);
    EnableEvent(D_8016B76C);
    EnableEvent(D_8016B770);
    EnableEvent(D_8016B774);
    EnableEvent(D_8016B778);
    EnableEvent(D_8016B77C);
    ExitCriticalSection();
}

/**
 * @brief Close the eight memory-card events opened by func_8014F2B0.
 * @note Teardown counterpart to func_8014F2B0; same critical-section wrapper and
 *       the same D_8016B760..D_8016B77C descriptor order.
 * @note The descriptors are not cleared afterwards, so the globals hold stale
 *       handles until the next func_8014F2B0 call overwrites them.
 * @see decomp.me (100%)
 */
void func_8014F46C(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(D_8016B760);
    CloseEvent(D_8016B764);
    CloseEvent(D_8016B768);
    CloseEvent(D_8016B76C);
    CloseEvent(D_8016B770);
    CloseEvent(D_8016B774);
    CloseEvent(D_8016B778);
    CloseEvent(D_8016B77C);
    ExitCriticalSection();
}

extern char D_8014057C[];
extern struct DIRENTRY D_8016B780[];
extern s32 D_8016B9D8;

/**
 * @brief Scan memory card slot 1 and record how many save files it holds.
 *
 * Delegates to func_8014F824, which globs "bu00:" + a wildcard through
 * firstfile/nextfile and fills the D_8016B780 directory-entry table. The
 * resulting file count is cached in D_8016B9D8.
 *
 * @note The redundant-looking @c D_8016B9D8=0 before the call is required to
 *       match: the target stores $zero in the jal delay slot, and dropping the
 *       statement scores 32.06%. It also leaves the count at 0 if the callee
 *       never returns (card removed mid-scan).
 * @note The "bu00:" path must be referenced as the pre-split rodata symbol
 *       D_8014057C, not as a string literal: a literal emits a fresh local
 *       rodata label and scores 99.38% (2 relocation rows).
 * @see decomp.me (100%)
 */
void func_8014F51C(void)
{
    D_8016B9D8 = 0;
    D_8016B9D8 = func_8014F824(D_8014057C, D_8016B780);
}

/**
 * @brief Bring memory card slot 1 up to a usable state, formatting it if needed.
 *
 * Queries the card, then dispatches on the func_8014F63C status code: an error
 * (1) or timeout (2) aborts immediately, and a newly inserted card (3) triggers
 * a reset/clear cycle before the load. After _card_load the status is polled
 * again; a 3 at that point means the card is unformatted, so _card_format is
 * attempted.
 *
 * @return 1 if the card is ready for use, 0 if it was rejected up front or the
 *         format attempt failed.
 * @note The status codes come from func_8014F63C, which maps them to the SwCARD
 *       events opened by func_8014F2B0: 0 EvSpIOE, 1 EvSpERROR, 2 EvSpTIMOUT,
 *       3 EvSpNEW.
 * @note Testing 1 and 2 in ONE @c || condition is required to match: gcc folds
 *       the pair into the target's @c addiu/sltiu range check, and splitting
 *       them into two separate @c if statements scores 86.85%. Writing the
 *       condition explicitly as @c (u32)(status-1)<2U is a measured non-factor
 *       (also 100%), so the readable form is kept.
 * @note Measured non-factor: collapsing the trailing status/format pair into a
 *       single @c && condition is also 100%.
 * @see decomp.me (100%)
 */
s32 func_8014F55C(void)
{
    s32 status;

    _card_info(0);
    status = func_8014F63C();
    if ((status == 1) || (status == 2))
    {
        return 0;
    }
    if (status == 3)
    {
        func_8014F7CC();
        _card_clear(0);
        func_8014F730();
    }
    func_8014F6D8();
    _card_load(0);
    if (func_8014F63C() == 3)
    {
        if (_card_format(0) == 0)
        {
            return 0;
        }
    }
    return 1;
}

extern char D_80140584[];
extern u8 D_80169760[];

/**
 * @brief Populate the card work buffer and write it out under the "bu00:HAND" name.
 * @note D_80140584 is the pre-split rodata string "bu00:HAND"; like the "bu00:"
 *       path in func_8014F51C it must be referenced as the symbol rather than a
 *       literal so the relocation targets the existing rodata.
 * @note D_80169760 is the shared card work buffer; func_8014FBCC fills it and
 *       func_8014F9EC commits it. Its size and layout are not yet known.
 * @see decomp.me (100%)
 */
void func_8014F5FC(void)
{
    func_8014FBCC(D_80169760);
    func_8014F9EC(D_80140584, D_80169760);
}

/**
 * @brief Block until one of the four SwCARD events fires and report which.
 *
 * Spins over the first four descriptors opened by func_8014F2B0, in the order
 * they were opened, and returns as soon as TestEvent reports one ready. Events
 * that are not ready leave the loop running, so this busy-waits until the card
 * driver signals something.
 *
 * @return 0 for EvSpIOE (operation completed), 1 for EvSpERROR, 2 for
 *         EvSpTIMOUT, 3 for EvSpNEW (card newly inserted / unformatted).
 * @note Comparing TestEvent's result against 1 explicitly is required to match:
 *       gcc pins the constant 1 in s0 and compares with @c bne against it, so
 *       the looser @c !=0 form scores 59.74%.
 * @note The four tests must be four symmetric @c if blocks. m2c reconstructs the
 *       third as an inverted test wrapping the fourth; that shape is 75.18%.
 * @note Measured non-factor: @c while(1) instead of @c for(;;) is also 100%.
 * @see decomp.me (100%)
 */
s32 func_8014F63C(void)
{
    for (;;)
    {
        if (TestEvent(D_8016B760) == 1)
        {
            return 0;
        }
        if (TestEvent(D_8016B764) == 1)
        {
            return 1;
        }
        if (TestEvent(D_8016B768) == 1)
        {
            return 2;
        }
        if (TestEvent(D_8016B76C) == 1)
        {
            return 3;
        }
    }
}

/**
 * @brief Drain the four SwCARD events by testing each one once.
 *
 * TestEvent clears an event's ready flag as a side effect, so calling it on all
 * four descriptors and discarding the results leaves them in a known-clear state.
 * func_8014F55C runs this immediately before _card_load so the status poll that
 * follows can only observe events raised by that load.
 *
 * @note Same descriptor order as func_8014F2B0 and func_8014F63C: EvSpIOE,
 *       EvSpERROR, EvSpTIMOUT, EvSpNEW.
 * @see decomp.me (100%)
 */
void func_8014F6D8(void)
{
    TestEvent(D_8016B760);
    TestEvent(D_8016B764);
    TestEvent(D_8016B768);
    TestEvent(D_8016B76C);
}

/**
 * @brief Block until one of the four HwCARD events fires and report which.
 *
 * Hardware-side twin of func_8014F63C: identical polling loop and identical
 * return mapping, but over the second group of descriptors opened by
 * func_8014F2B0 (HwCARD rather than SwCARD).
 *
 * @return 0 for EvSpIOE (operation completed), 1 for EvSpERROR, 2 for
 *         EvSpTIMOUT, 3 for EvSpNEW (card newly inserted / unformatted).
 * @note func_8014F55C calls this for its blocking side effect only, discarding
 *       the result, to let the hardware settle after _card_clear.
 * @note Re-measured on this function rather than inherited from func_8014F63C:
 *       the explicit @c ==1 compare is required (@c !=0 scores 59.74%), and the
 *       four symmetric @c if blocks are required (m2c's inverted-third-test
 *       shape scores 75.18%). Same figures as the SwCARD twin.
 * @see decomp.me (100%)
 */
s32 func_8014F730(void)
{
    for (;;)
    {
        if (TestEvent(D_8016B770) == 1)
        {
            return 0;
        }
        if (TestEvent(D_8016B774) == 1)
        {
            return 1;
        }
        if (TestEvent(D_8016B778) == 1)
        {
            return 2;
        }
        if (TestEvent(D_8016B77C) == 1)
        {
            return 3;
        }
    }
}

/**
 * @brief Drain the four HwCARD events by testing each one once.
 *
 * Hardware-side twin of func_8014F6D8. func_8014F55C runs this first in its
 * status-3 path, so the _card_clear that follows starts from a clean slate and
 * the func_8014F730 poll after it can only see events that clear raised.
 *
 * @note Same descriptor order as func_8014F2B0 and func_8014F730: EvSpIOE,
 *       EvSpERROR, EvSpTIMOUT, EvSpNEW.
 * @see decomp.me (100%)
 */
void func_8014F7CC(void)
{
    TestEvent(D_8016B770);
    TestEvent(D_8016B774);
    TestEvent(D_8016B778);
    TestEvent(D_8016B77C);
}

extern char D_80140590[];

/**
 * @brief Count the files on a memory card matching a path prefix.
 *
 * Builds "<path>*" in a local buffer and walks the card directory with
 * firstfile/nextfile, filling @p entry as it goes. Both BIOS calls return the
 * entry pointer they were given on success and NULL when the enumeration ends,
 * so the loop compares the result against the pointer rather than testing NULL.
 *
 * @param path Device path prefix, e.g. the "bu00:" string at D_8014057C.
 * @param entry Start of the caller's directory-entry table; one struct DIRENTRY
 *              is filled per file found, so it must have room for every match.
 * @return Number of files found; 0 if the card holds no match at all.
 * @note D_80140590 is the pre-split rodata string "*" (the wildcard suffix).
 * @note Inside the do/while, @c count must be incremented BEFORE @c entry:
 *       the reverse order scores 98.67%.
 * @note The target's trailing @c addiu s1,s1,-0x1 is NOT in the source. gcc
 *       speculatively puts the count increment in the @c beq delay slot, where
 *       it runs on both paths, then compensates on the exit path. m2c
 *       reconstructs that as a literal @c count-1 with a second variable; that
 *       shape is only 93.12%, and the clean loop below is 100%.
 * @see decomp.me (100%)
 */
s32 func_8014F824(char* path, struct DIRENTRY* entry)
{
    char pattern[0x80];
    s32 count;

    strcpy(pattern, path);
    strcat(pattern, D_80140590);
    count = 0;
    if (firstfile(pattern, entry) == entry)
    {
        do
        {
            count += 1;
            entry += 1;
        } while (nextfile(entry) == entry);
    }
    return count;
}

/**
 * @brief Read block 0 of a memory card and report whether it is formatted.
 *
 * Drains the four HwCARD events, forces a re-detect with _new_card(), reads the
 * first block into a local buffer, then runs the same four-way event poll as
 * func_8014F730 with the outcome in @c status. A non-zero status (error,
 * timeout, or newly inserted card) aborts. Otherwise the block is checked for
 * the "MC" signature that starts every formatted PlayStation card.
 *
 * @param chan Card channel / slot to probe, passed straight to _card_read.
 * @return 1 if the card is formatted, 0 if the "MC" magic is absent, -1 if the
 *         event poll reported anything other than completion.
 * @note NOT A MATCH - 97.54%. 80 insns vs the target's 81: the ONE missing
 *       instruction is `addu s4, v0, zero` at 0x38, the loop-base copy for
 *       D_8016B770. Everything else is a saved-register permutation that
 *       cascades from that copy not existing. Target map (m2c --reg-vars):
 *       status s0, chan s1, constant-1 s2, &D_8016B774 s3, &D_8016B770 s4;
 *       this build gets status s3, chan s2, constant-1 s0, &D_8016B774 s1.
 * @note @c new_var is required to match: assigning 0 to a local and returning
 *       it (instead of a bare `return 0`) is worth +5 exact rows and removes
 *       the spurious instruction the bare form emits. Reverting it scores
 *       93.28% with 4 `replace` rows and an extra wrong insn.
 * @note Cause is ALLOC-ORDER: the constant-1 pseudo is hoisted to the loop
 *       preheader with 9 loop-weighted refs (pri 5400) and outranks status
 *       (5 refs, pri 2500), so it takes s0 first. Underneath that is a
 *       SCHED-LUID ordering problem - the target materializes the constant in
 *       the 2nd drain delay slot and status=3 in the 4th, mine has them
 *       swapped, and the preheader-created constant can never get a luid
 *       earlier than a status=3 written before the loop.
 * @note Do not "fix" this with a shared zero local or a short-typed flag. Those
 *       reach 96.21% but emit a saved register where the target uses hardware
 *       $zero, and xori/bnez where the target has bne. See
 *       working/func_8014F8A8/status.md for the full probe log.
 * @see decomp.me (97.54%)
 */
s32 func_8014F8A8(s32 chan)
{
    u8 header[0x80];
    s32 status;
    s32 new_var;

    bzero(header, 0x80);
    TestEvent(D_8016B770);
    TestEvent(D_8016B774);
    TestEvent(D_8016B778);
    TestEvent(D_8016B77C);
    status = 3;
    _new_card();
    new_var = 0;
    _card_read(chan, 0, header);
    while (1)
    {
        if (TestEvent(D_8016B770) == 1)
        {
            status = 0;
            break;
        }
        if (TestEvent(D_8016B774) == 1)
        {
            status = 1;
            break;
        }
        if (TestEvent(D_8016B778) == 1)
        {
            status = 2;
            break;
        }
        if (TestEvent(D_8016B77C) == 1)
        {
            break;
        }
    }
    if (status != 0)
    {
        return -1;
    }
    if (header[0] != 'M')
    {
        return 0;
    }
    if (header[1] == 'C')
    {
        return 1;
    }
    return new_var;
}

extern u8 D_80140594[];
extern u8 D_80169560[];
extern u8 D_80169563;

/**
 * @brief Create a memory-card save file and write one block of save data to it.
 *
 * Fills the 0x200-byte card header at D_80169560 - the "SC" magic, icon/display
 * byte 0x13, the block count, and the Shift-JIS title "\x90\xB9\x8C\x95\x83Z\x81[\x83u\x83f\x81[\x83^"
 * ("Seiken save data") - zeroes the reserved tail, then stamps that header over
 * the front of the caller's buffer. The file is created with open(), closed,
 * reopened for writing, and the whole 8 KB block is written.
 *
 * @param name Card path to create, e.g. the "bu00:HAND" string at D_80140584.
 * @param buf  Caller's 8 KB save buffer; its first 0x200 bytes are overwritten
 *             with the header before the write.
 * @return 1 if the file was created and fully written, 0 if either open failed
 *         or the write was short.
 * @note The create flags are (blocks << 16) | 0x200 - the PS1 BIOS takes the
 *       file's block count in the upper half of open()'s mode argument.
 * @note @c blocks must be a SHARED local feeding both the header byte and the
 *       write size (`blocks << 13`). The target keeps the constant 1 in s3 and
 *       derives 0x2000 from it; writing the two constants independently scores
 *       93.59%. Measured non-factors: `blocks * 0x2000` for the shift, and
 *       inlining the size into the write/compare (both 100%).
 * @note The header byte at +3 is written through the D_80169560 array but read
 *       back through the separate symbol D_80169563. That asymmetry is required:
 *       routing the store through D_80169563 too costs an insn and scores
 *       94.17%, and reading via D_80169560[3] emits a `+0x3` relocation instead
 *       of the target's own symbol (99.88%).
 * @note Both copies are gcc's inlined memcpy on byte-aligned char pointers -
 *       hence the lwl/lwr pairs and, for the 0x200 copy into a pointer
 *       parameter, the runtime (src|dst)&3 alignment test with two loop bodies.
 *       m2c cannot reconstruct these and reports M2C_ERROR on every lwr.
 * @note Statement order matters: swapping the bzero and the 0x200 memcpy scores
 *       88.71%.
 * @see decomp.me (100%)
 */
s32 func_8014F9EC(char* name, void* buf)
{
    s32 blocks;
    s32 fd;
    s32 size;

    D_80169560[0] = 'S';
    D_80169560[1] = 'C';
    D_80169560[2] = 0x13;
    blocks = 1;
    D_80169560[3] = blocks;
    memcpy(&D_80169560[4], D_80140594, 0x11);
    bzero(&D_80169560[0x44], 0x1C);
    memcpy(buf, D_80169560, 0x200);
    fd = open(name, (D_80169563 << 16) | 0x200);
    if (fd == -1)
    {
        return 0;
    }
    close(fd);
    fd = open(name, 2);
    if (fd == -1)
    {
        return 0;
    }
    size = blocks << 13;
    if (write(fd, buf, size) != size)
    {
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}

extern u8 D_801405A8[];

/**
 * @brief Fill the card work buffer with placeholder save contents.
 *
 * Copies the 5-byte string "TEST" (including its terminator) from D_801405A8 to
 * the front of @p buf. Paired with func_8014F9EC by func_8014F5FC, which fills
 * the buffer here and then commits it to the card.
 *
 * @param buf Card work buffer to fill; only the first 5 bytes are touched.
 * @note This is placeholder/debug content - the shipped save path writes the
 *       literal text "TEST" as its payload, with the real header supplied
 *       separately by func_8014F9EC.
 * @note The source string must be the rodata symbol, not a literal: a "TEST"
 *       literal emits a fresh local label and scores 99.00%. It must also be
 *       memcpy and not strcpy (strcpy emits a real call, 0.00%). Measured
 *       non-factor: declaring @p buf as u8* instead of void* is also 100%.
 * @note gcc inlines the 5-byte copy as an lwl/lwr + lb pair, with no runtime
 *       alignment test - unlike the 0x200 copy in func_8014F9EC, which is large
 *       enough to get the (src|dst)&3 check and two loop bodies.
 * @see decomp.me (100%)
 */
void func_8014FBCC(void* buf)
{
    memcpy(buf, D_801405A8, 5);
}
