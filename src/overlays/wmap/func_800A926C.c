#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DD7B4[];
extern u8 D_800DDE54[];
extern u8 D_800DE39C[];
extern u8 D_800DF378[];
extern u8 D_800E0354[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern u8 *D_8011CF30;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A926C(void)
{
    func_800A8AA8(0x113D);
    func_800A8AF0(0x113E);
    func_800A8B38(0x113F);
    cdrom_queue_read(0x1140, D_8011D538);
    cdrom_queue_read(0x1141, D_8011F538);
    cdrom_queue_read(0x1142, D_80121538);
    D_8011CF1C = D_800DD7B4;
    D_8011CF24 = D_800DDE54;
    D_8011CF28 = D_800DE39C;
    D_8011CF2C = D_800DF378;
    D_8011CF30 = D_800E0354;
    cdrom_queue_read(0x1143, D_800DCF18);
}
