#include "common.h"

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
extern void func_801429A4(void);
extern void func_800AA02C(void);

void func_801428F0(void)
{
    D_801451D8.draw = func_801429A4;
    D_801451D4 = 1;
    D_801451D0 = 0;
    D_801451D8.state.bits.state = 1;
    D_801451D8.state.bits.phase = 1;
    D_801451D8.state.bits.kind = 0x40;
    D_801451D8.state.bits.y = 0x70;
    D_801451D8.size.bits.flag = 0;
    D_801451D8.size.bits.size = 0x20;
    D_801451D8.state.word = (D_801451D8.state.word & 0xFFFFFF) | 0xC0000000;
    func_800AA02C();
}
