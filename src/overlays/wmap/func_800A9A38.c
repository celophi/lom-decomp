#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DDEF4[];
extern u8 D_800DEED0[];
extern u8 D_800E2B1C[];
extern u8 D_800E2DD8[];
extern u8 D_800E33AC[];
extern u8 D_800E3980[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern u8 *D_8011CF30;
extern u8 *D_8011CF34;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9A38(void)
{
    func_800A8AA8(0x11A0);
    func_800A8AF0(0x11A1);
    func_800A8B38(0x11A2);
    cdrom_queue_read(0x11A3, D_8011D538);
    cdrom_queue_read(0x11A4, D_8011F538);
    cdrom_queue_read(0x11A5, D_80121538);
    D_8011CF1C = D_800DDEF4;
    D_8011CF24 = D_800DEED0;
    D_8011CF28 = D_800E2B1C;
    D_8011CF2C = D_800E2DD8;
    D_8011CF30 = D_800E33AC;
    D_8011CF34 = D_800E3980;
    cdrom_queue_read(0x11A6, D_800DCF18);
}
