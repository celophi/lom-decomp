#include "common.h"

typedef struct
{
    s32 word;
} CardaAttrWord;

typedef struct
{
    CardaAttrWord attr;
    s32 unk4;
    s32 unk8;
} CardaPacket;

typedef CardaPacket CardaElement;

extern s32 D_80165F80;

CardaElement *func_80142614(void)
{
    CardaPacket *p;
    s32 i;

    p = (CardaPacket *)&D_80165F80;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->attr.word & 7) == 0)
        {
            p->attr.word = (p->attr.word & ~7) | 1;
            return (CardaElement *)p;
        }
    }
    return (CardaElement *)&D_80165F80;
}
