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

extern ShopElementState D_801451D8;
extern s32 D_80145238;
extern s32 D_8014523C;
extern void func_80142284(void);
extern void func_800AA02C(void);

void func_80140CA4(s32 arg0)
{
    D_8014523C = arg0;
    D_801451D8.draw = func_80142284;
    D_80145238 = 1;
    D_801451D8.state.bits.state = 1;
    D_801451D8.state.bits.phase = 1;
    D_801451D8.state.bits.kind = 0x20;
    D_801451D8.state.bits.y = 0x70;
    D_801451D8.size.bits.flag = 1;
    D_801451D8.size.bits.size = 0x10;
    D_801451D8.state.word &= 0xFFFFFF;
    func_800AA02C();
}
