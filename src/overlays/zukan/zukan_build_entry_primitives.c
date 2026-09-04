#include "common.h"

typedef struct
{
    u8 pad[8];
    u16 field_8;
    u16 field_A;
} ZukanEntryRecord;

extern ZukanEntryRecord D_80157420[];

void func_801418A4(u8 *arg0)
{
    u8 *dst;
    s32 i;
    s32 cur;
    s32 one;
    volatile s32 pad[2];

    dst = arg0 + 0x28;
    i = 0;
    cur = *(s32 *)(arg0 + 0x4040);
    do {
        cur = func_80141988(cur, dst, i, D_80157420[i].field_8,
                           D_80157420[i].field_A, 0);
        i++;
    } while (i < 6);

    dst = arg0 + 0x3C;
    i = 6;
    one = 1;
    do {
        cur = func_80141988(cur, dst, i, D_80157420[i].field_8,
                           D_80157420[i].field_A, one);
        i++;
    } while (i < 0x15);

    *(s32 *)(arg0 + 0x4040) = func_801424EC(cur, arg0 + 0x2C);
}
