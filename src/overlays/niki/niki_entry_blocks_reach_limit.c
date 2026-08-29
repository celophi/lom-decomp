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

extern s32 D_80164B78;
extern s32 D_80164B70;
extern char D_80165018[];

s32 func_80144CC8(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (D_80164B78 > 0)
    {
        offset = D_80164B70 * 0x320;
        do
        {
            do {
                sum += ((Entry *)((u8 *)D_80165018 + offset))->size / 8192;
            } while (0);
            i++;
            offset += 0x28;
        } while (i < D_80164B78);
    }
    return sum >= 0xE;
}
