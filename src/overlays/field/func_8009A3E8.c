#include "common.h"

s32 cdrom_queue_read(u16 id, s32 dest);
extern s32 D_8010D030;
extern s32 D_8010D038;
extern s32 D_8010D040[];
extern s32 D_8010D080;

void func_8009A3E8(void)
{
    s32 count;
    s32 i;

    if (D_8010D030 == 0) {
        D_8010D080 = -1;
        return;
    }

    if (cdrom_queue_read((u16)D_8010D040[0], D_8010D038) >= 0) {
        i = 0;
        D_8010D080 = D_8010D040[0];
        count = D_8010D030 - 1;
        if (count > 0) {
            do {
                D_8010D040[i] = D_8010D040[i + 1];
                i++;
            } while (i < count);
        }
        D_8010D030 -= 1;
    }
}
