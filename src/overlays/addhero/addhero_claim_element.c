#include "common.h"

typedef struct
{
    s32 word;
} AddheroAttrWord;

typedef struct
{
    AddheroAttrWord attr;
    s32 unk4;
    s32 unk8;
} AddheroPacket;

typedef AddheroPacket AddheroElement;

extern s32 D_80160940;

AddheroElement *func_80141EAC(void)
{
    AddheroPacket *p;
    s32 i;

    p = (AddheroPacket *)&D_80160940;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->attr.word & 7) == 0)
        {
            p->attr.word = (p->attr.word & ~7) | 1;
            return (AddheroElement *)p;
        }
    }
    return (AddheroElement *)&D_80160940;
}
