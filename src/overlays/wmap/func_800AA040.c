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
extern u8 D_80123538[];
extern u8 D_80125538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;

/** @brief Divide the effect buffer and queue its animation and texture reads. */
void func_800AA040(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    D_8011CF2C = D_8011CF28 + 0x2000;
    func_800A8AA8(0x11E0);
    func_800A8AF0(0x11E1);
    func_800A8B38(0x11E2);
    cdrom_queue_read(0x11E3, D_800DCF18);
    cdrom_queue_read(0x11E4, D_8011CF1C);
    cdrom_queue_read(0x11E5, D_8011CF24);
    cdrom_queue_read(0x11E6, D_8011CF28);
    cdrom_queue_read(0x11E7, D_8011CF2C);
    cdrom_queue_read(0x11E8, D_8011D538);
    cdrom_queue_read(0x11E9, D_8011F538);
    cdrom_queue_read(0x11EA, D_80121538);
    cdrom_queue_read(0x11EB, D_80123538);
    cdrom_queue_read(0x11EC, D_80125538);
}
