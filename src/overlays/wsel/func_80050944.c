#include "common.h"
typedef struct { s16 x; s16 y; } WselPoint;
extern u8 D_800C6720[];
extern WselPoint D_800CA8B8, D_800CA8BC, D_800CA8C8, D_800CA8CC;
extern s32 D_800CA8DC, D_800CA8E0;
void func_80050944(void)
{
    s32 x_step0, y_step0, x_step1, y_step1;
    if (D_800CA8DC != 0) {
        WselPoint *current = &D_800CA8B8;
        x_step0 = (D_800CA8C8.x - current->x) / D_800CA8DC;
        y_step0 = (D_800CA8C8.y - current->y) / D_800CA8DC;
        D_800CA8DC -= 1;
        current->x += x_step0;
        current->y += y_step0;
    } else { D_800CA8B8.x = D_800CA8C8.x; D_800CA8B8.y = D_800CA8C8.y; }
    if (D_800CA8E0 != 0) {
        WselPoint *current = &D_800CA8BC;
        x_step1 = (D_800CA8CC.x - current->x) / D_800CA8E0;
        y_step1 = (D_800CA8CC.y - current->y) / D_800CA8E0;
        D_800CA8E0 -= 1;
        current->x += x_step1;
        current->y += y_step1;
    } else { D_800CA8BC.x = D_800CA8CC.x; D_800CA8BC.y = D_800CA8CC.y; }
    { u8 *table = D_800C6720; *(s16 *)(table + 0x44) = D_800CA8BC.x - 0xB; *(s16 *)(table + 0x46) = D_800CA8BC.y - 0xB; *(u16 *)(table + 0x14) = D_800CA8B8.x; *(u16 *)(table + 0x16) = D_800CA8B8.y; }
}
