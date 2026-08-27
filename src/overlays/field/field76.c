#include "common.h"

typedef struct
{
    u32 word;
    u8 pad[0x14 - 4];
} UnkEntry80122828;

typedef struct
{
    s32 flags;  /* 0x00 */
    u8 pad4[0x10];
} RecADEEC;

extern s32 D_8012298C;
extern UnkEntry80122828 D_80122828[];

void func_800ADEB0(void)
{
    UnkEntry80122828 *p;
    s32 i;

    D_8012298C = 0;
    p = D_80122828;
    for (i = 0; i < 8; i++)
    {
        p->word &= ~7;
        p++;
    }
}

extern RecADEEC D_80122828[];

s32 func_800ADEEC(void)
{
    RecADEEC *p;
    s32 i;
    s32 x;

    p = D_80122828;
    for (i = 0; i < 8; i++, p++)
    {
        x = p->flags & 0x7;
        if (x != 0)
        {
            if (x != 2)
            {
                return 1;
            }
        }
    }
    return 0;
}

void func_800ADF34(void)
{
    RecADEEC *p;
    s32 i;
    u32 x;

    p = D_80122828;
    for (i = 0; i < 8; i++, p++) {
        x = p->flags;
        if (x & 7) {
            p->flags = (((x & ~7) | 3) & ~0x78) | 0x40;
        }
    }
}
