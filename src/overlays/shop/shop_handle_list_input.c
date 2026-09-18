#include "common.h"

typedef struct
{
    u16 id;
    u16 count;
    s32 value;
} ShopEntry;

typedef struct
{
    union
    {
        u32 word;
        struct
        {
            unsigned state : 3;
            unsigned phase : 4;
            unsigned kind : 9;
            unsigned y : 8;
            unsigned code : 8;
        } bits;
    } state;
    union
    {
        u32 word;
        struct
        {
            unsigned flag : 1;
            unsigned size : 8;
            unsigned rest : 23;
        } bits;
    } size;
    void (*draw)(void);
} ShopElementState;

extern s32 D_8012271C;
extern s32 D_80122988;
extern s32 D_801451C4;
extern s32 D_801451C8;
extern s32 D_801451D4;
extern ShopElementState D_801451D8[];
extern s32 D_80145238;
extern s32 D_80145240;
extern s32 D_80145248;
extern s32 D_8014524C;
extern s32 D_80145250;
extern s32 D_80145CD8;
extern s32 D_80145CDC;
extern s32 D_80145CE0;

extern void func_80067F28(void);
extern void func_800A3938();
extern void func_800A8FB4(void);
extern s32 func_800A9060(void);
extern void func_80140CA4(s32 arg0);
extern void func_8014234C(void);
extern void func_801428F0(void);

/**
 * @brief Handle shop list navigation, quantity changes, purchases, and cancellation.
 * @return Result is unspecified; no value is explicitly returned.
 */
s32 func_80140838(void)
{
    ShopElementState *element;
    ShopEntry *entry;
    s32 row_count;
    s32 last_row;
    s32 target_y;
    s32 current_scroll;
    s32 delta;
    s32 state;
    s32 i;

    state = D_801451D8[1].state.word & 7;
    if (state == 0)
    {
        D_801451C8 = 1;
    }
    if (state != 2)
    {
        return;
    }

    if (D_80145238 != 0)
    {
        if ((D_80122988 & 0x260) == 0)
        {
            return;
        }
        if ((D_801451D8[0].state.word & 7) == 0)
        {
            return;
        }
        D_801451D8[0].state.word = (((D_801451D8[0].state.word & ~7U) | 3U) & ~0x78U) | 0x40U;
        return;
    }

    if ((D_801451D4 != 0) || (D_80145CE0 != 0))
    {
        return;
    }

    i = 1;
    if (D_80122988 & 8)
    {
        D_80122988 = 0x4000;
        i = 7;
    }
    if (D_80122988 & 4)
    {
        D_80122988 = 0x1000;
        i = 7;
    }

    if (i != 0)
    {
        s32 backward_input;
        s32 forward_input;

        backward_input = D_80122988 & 0x1000;
        row_count = D_80145CD8;
        last_row = row_count - 1;
        forward_input = D_80122988 & 0x4000;
        do
        {
            if (backward_input)
            {
                D_80145240 = 1;
                D_80145CDC -= 1;
                if (D_80145CDC < 0)
                {
                    D_80145CDC = last_row;
                }
            }
            if (forward_input)
            {
                D_80145240 = 1;
                D_80145CDC += 1;
                if (D_80145CDC >= row_count)
                {
                    D_80145CDC = 0;
                }
            }
            if ((D_80145CDC == last_row) || (D_80145CDC == 0))
            {
                i = 1;
            }
            i -= 1;
        } while (i != 0);
    }

    if (D_80122988 & 0x5000)
    {
        func_800A3938(0x7D, 0x80);
        target_y = D_80145CDC << 4;
        current_scroll = D_80145248;
        delta = target_y - current_scroll;
        if (delta >= 0x65)
        {
            D_8014524C = target_y - 0x60;
            D_80145CE0 = 4;
        }
        if (delta < 0)
        {
            D_8014524C = target_y;
            D_80145CE0 = 4;
        }
    }

    if (D_80122988 & 0x8000)
    {
        if (D_80145240 >= 2)
        {
            func_800A3938(0x7D, 0x80);
            D_80145240 -= 1;
        }
    }

    if (D_80122988 & 0x2000)
    {
        ShopEntry *quantity_entry;

        quantity_entry = (ShopEntry *)((D_80145CDC * sizeof(ShopEntry)) + D_80145250);
        if (quantity_entry->count != 0)
        {
            if (D_80145240 < quantity_entry->count)
            {
                func_800A3938(0x7D, 0x80);
                D_80145240 += 1;
            }
        }
        else
        {
            if (D_80145240 < 0x63)
            {
                func_800A3938(0x7D, 0x80);
                D_80145240 += 1;
            }
        }
    }

    if (D_80122988 & 0x220)
    {
        entry = (ShopEntry *)((D_80145CDC * sizeof(ShopEntry)) + D_80145250);
        if (entry->id != 0xFFFF)
        {
            if (D_801451C4 != 0)
            {
                if (*(u32 *)(D_8012271C + 0x2C) < (u32)(entry->value * D_80145240))
                {
                    goto insufficient_funds;
                }
                if (entry->id & 0x8000)
                {
                    if (func_800A9060() == 0)
                    {
                        goto invalid_purchase;
                    }
                    goto valid_purchase;
                }
                if (*(u8 *)(D_8012271C + entry->id + 0x25E0) < 0x63)
                {
                    goto valid_purchase;
                }

invalid_purchase:
                func_800A3938(0x78, 0x80);
                func_80140CA4(0);
                goto check_cancel;

valid_purchase:
                func_8014234C();
                goto check_cancel;

insufficient_funds:
                func_800A3938(0x78, 0x80);
                goto check_cancel;
            }
            func_801428F0();
        }
    }

check_cancel:

    if (D_80122988 & 0x40)
    {
        func_800A3938(0x7F, 0x80);
        if (D_801451C4 == 0)
        {
            func_800A8FB4();
        }
        func_80067F28();
        element = D_801451D8;
        for (i = 0; i < 8; i++, element++)
        {
            if (element->state.word & 7)
            {
                element->state.word = (((element->state.word & ~7U) | 3U) & ~0x78U) | 0x40U;
            }
        }
    }
}
