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

extern s32 D_801451D0;
extern s32 D_801451D4;
extern ShopElementState D_801451D8;
extern s32 D_80145238;
extern s32 D_80145240;
extern s32 D_80145244;
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
extern void func_80142284(void);

s32 func_801404A8(s32 arg0, s32 count, s32 src, s32 arg3)
{
    ShopElementState *element;
    ShopRect pos;
    s32 i;

    i = 0;
    D_801451D4 = 0;
    D_801451D0 = 0;
    if (count > 0)
    {
        s32 dst = (s32)D_80145258;
        do
        {
            *(u16 *)(dst + 0) = *(u16 *)(src + 0);
            i++;
            *(u16 *)(dst + 2) = *(u16 *)(src + 2);
            *(s32 *)(dst + 4) = *(s32 *)(src + 4);
            src += 8;
            dst += 8;
        } while (i < count);
    }
    D_80145250 = D_80145258;
    D_80145CD8 = count;
    D_80145244 = arg3;
    D_80145CE0 = 0;
    D_8014524C = 0;
    D_80145248 = 0;
    D_80145240 = 1;
    D_80145CDC = 0;
    func_80140D6C();

    element = func_80140DAC();
    element->draw = func_80142284;
    element->state.bits.phase = 1;
    element->state.bits.kind = 0x20;
    element->size.bits.flag = 1;
    element->size.bits.size = 0x10;
    element->state.bits.y = 0x70;
    element->state.word &= 0xFFFFFF;

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
    D_80145238 = 0;
    element->state.word = (element->state.word & 0xFFFFFF) | 0x70000000;
    D_801451D8.state.word &= ~7;
    return arg0;
}
