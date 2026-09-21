#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800ECF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 D_8011D538[];
extern u8 D_8011F538[];

/** @brief Queue the world-map effect resource set. */
void func_800A96AC(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    func_800A8AA8(0x1176);
    func_800A8AF0(0x1177);
    func_800A8B38(0x1178);
    cdrom_queue_read(0x1179, D_8011D538);
    cdrom_queue_read(0x117A, D_8011F538);
    cdrom_queue_read(0x117B, D_800DCF18);
    cdrom_queue_read(0x117C, D_8011CF1C);
    cdrom_queue_read(0x117D, D_8011CF24);
    cdrom_queue_read(0x117E, D_8011CF28);
}
