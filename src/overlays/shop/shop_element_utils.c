#include "common.h"

typedef struct
{
    s32 word;
} ShopAttr;

typedef struct
{
    ShopAttr attr;
    s32 unk4;
    s32 unk8;
} ShopPacket;

typedef ShopPacket ShopElement;

extern s32 g_menu_element_counter;
extern ShopElement D_801451D8;
extern void func_80140E00(void);

void func_80140D4C(void)
{
    func_80140E00();
}

void func_80140D6C(void)
{
    ShopPacket *p;
    s32 i;

    g_menu_element_counter = 0x20;
    p = (ShopPacket *)&D_801451D8;
    for (i = 0; i < 8; i++)
    {
        p->attr.word &= ~7;
        p++;
    }
}

ShopElement *func_80140DAC(void)
{
    ShopPacket *p;
    s32 i;

    p = (ShopPacket *)&D_801451D8;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->attr.word & 7) == 0)
        {
            p->attr.word = (p->attr.word & ~7) | 1;
            return (ShopElement *)p;
        }
    }
    return &D_801451D8;
}
