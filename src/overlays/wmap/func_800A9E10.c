#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DEF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9E10(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11CC);
    func_800A8AF0(0x11CD);
    func_800A8B38(0x11CE);
    cdrom_queue_read(0x11CF, D_8011D538);
    cdrom_queue_read(0x11D0, D_8011F538);
    cdrom_queue_read(0x11D1, D_80121538);
    cdrom_queue_read(0x11D2, D_800DCF18);
    cdrom_queue_read(0x11D3, D_8011CF1C);
    cdrom_queue_read(0x11D4, D_8011CF24);
}
