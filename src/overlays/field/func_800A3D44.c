#include "common.h"

extern u8 *D_8010D038;
extern u8 D_8011BF00[];
void cdrom_queue_read(s32 id, void *dst);
void cdrom_wait_queue_empty(void);
void func_80022ED8(void *p, s32 index, s32 one);

void func_800A3D44(s32 arg0, s32 arg1)
{
    u8 *dst;
    u8 *base;
    u8 *src;
    u8 *end;
    u8 *dst_cursor;

    if (arg1 != -2) {
        base = D_8011BF00;
        dst = base + arg0 * 0x1A00;
        *(s32 *)dst = 0;
        if (arg1 != -1) {
            arg1 += 0x83;
            src = D_8010D038;
            cdrom_queue_read(arg1 & 0xFFFF, src);
            cdrom_wait_queue_empty();
            end = src + *(s32 *)(src + (*(s32 *)src * 4));
            dst_cursor = dst;
            if (src != end) {
                do {
                    *dst_cursor++ = *src++;
                } while (src != end);
            }
            func_80022ED8(end, arg0, 1);
        }
    }
}
