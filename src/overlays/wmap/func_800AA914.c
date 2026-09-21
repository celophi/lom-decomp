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
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];

/** @brief Queue the world-map effect resource set. */
void func_800AA914(void)
{
    D_8011CF1C = D_800DDF18;
    D_8011CF24 = D_8011CF1C + 0x1000;
    D_8011CF28 = D_8011CF24 + 0x6000;
    func_800A8AA8(0x124B);
    func_800A8AF0(0x124C);
    func_800A8B38(0x124D);
    cdrom_queue_read(0x124E, D_8011D538);
    cdrom_queue_read(0x124F, D_8011F538);
    cdrom_queue_read(0x1250, D_80121538);
    cdrom_queue_read(0x1251, D_800DCF18);
    cdrom_queue_read(0x1252, D_8011CF1C);
    cdrom_queue_read(0x1253, D_8011CF24);
    cdrom_queue_read(0x1254, D_8011CF28);
}
