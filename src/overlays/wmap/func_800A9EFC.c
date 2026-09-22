#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800E0F18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9EFC(void)
{
    D_8011CF1C = D_800E0F18;
    D_8011CF24 = D_8011CF1C + 0x4000;
    D_8011CF28 = D_8011CF24 + 0x4000;
    D_8011CF2C = D_8011CF28 + 0x4000;
    func_800A8AA8(0x11D5);
    func_800A8AF0(0x11D6);
    func_800A8B38(0x11D7);
    cdrom_queue_read(0x11D8, D_8011D538);
    cdrom_queue_read(0x11D9, D_8011F538);
    cdrom_queue_read(0x11DA, D_80121538);
    cdrom_queue_read(0x11DB, D_800DCF18);
    cdrom_queue_read(0x11DC, D_8011CF1C);
    cdrom_queue_read(0x11DD, D_8011CF24);
    cdrom_queue_read(0x11DE, D_8011CF28);
    cdrom_queue_read(0x11DF, D_8011CF2C);
}
