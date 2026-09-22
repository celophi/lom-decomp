#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DEF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 D_80123538[];

/** @brief Queue the world-map effect resource set. */
void func_800AA2EC(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x4000;
    D_8011CF2C = D_8011CF28 + 0x4000;
    func_800A8AA8(0x11F9);
    func_800A8AF0(0x11FA);
    func_800A8B38(0x11FB);
    cdrom_queue_read(0x11FC, D_8011D538);
    cdrom_queue_read(0x11FD, D_8011F538);
    cdrom_queue_read(0x11FE, D_80121538);
    cdrom_queue_read(0x11FF, D_80123538);
    cdrom_queue_read(0x1200, D_800DCF18);
    cdrom_queue_read(0x1201, D_8011CF1C);
    cdrom_queue_read(0x1202, D_8011CF24);
    cdrom_queue_read(0x1203, D_8011CF28);
    cdrom_queue_read(0x1204, D_8011CF2C);
}
