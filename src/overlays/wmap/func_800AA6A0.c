#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DDF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];

/** @brief Queue the world-map effect resource set. */
void func_800AA6A0(void)
{
    D_8011CF1C = D_800DDF18;
    D_8011CF24 = D_8011CF1C + 0x1000;
    D_8011CF28 = D_8011CF24 + 0x1000;
    D_8011CF2C = D_8011CF28 + 0x10800;
    func_800A8AA8(0x1233);
    func_800A8AF0(0x1234);
    func_800A8B38(0x1235);
    cdrom_queue_read(0x1236, D_8011D538);
    cdrom_queue_read(0x1237, D_8011F538);
    cdrom_queue_read(0x1238, D_80121538);
    cdrom_queue_read(0x1239, D_800DCF18);
    cdrom_queue_read(0x123A, D_8011CF1C);
    cdrom_queue_read(0x123B, D_8011CF24);
    cdrom_queue_read(0x123C, D_8011CF28);
    cdrom_queue_read(0x123D, D_8011CF2C);
}
