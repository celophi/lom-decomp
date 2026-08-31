#include "common.h"

typedef struct
{
    s16 id;
    s16 count;
    s32 value;
} ShopEntry;

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} ShopRect;

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

extern void *D_8012271C;
extern u16 D_80144FB4[];
extern s32 D_801451D0;
extern s32 D_801451D4;
extern ShopElementState D_801451D8;
extern s32 D_80145238;
extern s32 D_80145240;
extern void *D_80145244;
extern s32 D_80145248;
extern s32 D_8014524C;
extern ShopEntry *D_80145250;
extern ShopEntry D_80145258[];
extern s32 D_80145CD8;
extern s32 D_80145CDC;
extern s32 D_80145CE0;
extern void func_80140D6C();
extern ShopElementState *func_80140DAC();
extern void func_801414F8(void);
extern void func_801415F4(void);
extern void func_801419D4(void);
extern void func_801420B8(void);

/**
 * @brief Build the default shop entry list and seed the shop UI elements.
 *
 * Taken when the custom-list flag (D_801451C4) is zero. Scans the resource
 * block at D_8012271C for sellable items, populates D_80145258 with an entry
 * per non-empty slot, then registers the four shop UI draw elements (money
 * value, list body, mode glyph, and header).
 *
 * @param arg0 Pass-through cursor/state value returned to the caller.
 * @return arg0 unchanged.
 * @see shop_setup_custom_list (func_801404A8), the non-zero-flag counterpart.
 */
s32 func_80140164(s32 arg0)
{
    ShopElementState *element;
    ShopRect pos;
    s32 count;
    s32 i;

    count = 0;
    {
        u8 *src;

        D_801451D4 = 0;
        D_801451D0 = 0;
        src = (u8 *)D_8012271C + 0xCE0;
        while (count < 0x64)
        {
            if (*(u8 *)src == 0)
            {
                break;
            }
            D_80145258[count].id = count - 0x8000;
            D_80145258[count].count = 1;
            D_80145258[count].value = *(s32 *)(src + 0x34);
            count++;
            src += 0x40;
        }
    }

    i = 0;
    {
        u8 *base = (u8 *)D_8012271C;
        u16 *values = D_80144FB4;
        do
        {
            u8 *entry = base + i;
            if (*(u8 *)(entry + 0x25E0) != 0)
            {
                D_80145258[count].id = i;
                D_80145258[count].count = *(u8 *)(entry + 0x25E0);
                D_80145258[count].value = *values;
                count++;
            }
            i++;
            values++;
        } while (i < 0x100);
    }

    D_80145250 = D_80145258;
    D_80145CD8 = count;
    D_80145CE0 = 0;
    D_8014524C = 0;
    D_80145248 = 0;
    D_80145240 = 1;
    D_80145CDC = 0;
    D_80145244 = (u8 *)D_8012271C + 0xCE0;
    func_80140D6C();

    D_801451D8.state.bits.state = 1;

    element = func_80140DAC();
    element->draw = func_801414F8;
    element->state.bits.phase = 1;
    element->state.bits.kind = 0xA0;
    element->size.bits.flag = 0;
    element->size.bits.size = 0x14;
    element->state.bits.y = 0x20;
    element->state.word = (element->state.word & 0xFFFFFF) | 0x7A000000;

    element = func_80140DAC();
    element->draw = func_801415F4;
    element->state.bits.phase = 1;
    element->state.bits.kind = 8;
    element->size.bits.flag = 1;
    element->size.bits.size = 0x74;
    element->state.bits.y = 0x38;
    element->state.word = (element->state.word & 0xFFFFFF) | 0x26000000;

    element = func_80140DAC();
    element->draw = func_801419D4;
    element->state.bits.phase = 1;
    element->state.bits.kind = 0xA;
    element->size.bits.flag = 1;
    element->size.bits.size = 0x24;
    element->state.bits.y = 0xB0;
    element->state.word = (element->state.word & 0xFFFFFF) | 0x2C000000;

    element = func_80140DAC();
    element->draw = func_801420B8;
    element->state.bits.phase = 1;
    element->state.bits.kind = 0x20;
    element->state.bits.y = 0x20;
    element->size.bits.flag = 0;
    element->size.bits.size = 0x14;
    element->state.word = (element->state.word & 0xFFFFFF) | 0x70000000;
    D_80145238 = 0;
    D_801451D8.state.word &= ~7;
    return arg0;
}
