#include "common.h"

typedef struct
{
    char name[20];
    s32 attr;
    s32 size;
    void *next;
    s32 head;
    char system[4];
} Entry;

extern s32 D_801609A4;
extern s32 D_801609A8;
extern char D_80164B60[];

s32 func_80144AF8(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (D_801609A4 > 0)
    {
        offset = D_801609A8 * 0x320;
        do
        {
            do {
                sum += ((Entry *)((u8 *)D_80164B60 + offset))->size / 8192;
            } while (0);
            i++;
            offset += 0x28;
        } while (i < D_801609A4);
    }
    return sum >= 0xE;
}
