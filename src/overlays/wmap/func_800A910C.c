#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DEF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern u8 *D_8011CF30;
extern u8 *D_8011CF34;

/** @brief Divide the effect buffer and queue its animation and texture reads. */
void func_800A910C(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    D_8011CF2C = D_8011CF28 + 0x2000;
    D_8011CF30 = D_8011CF2C + 0x2000;
    D_8011CF34 = D_8011CF30 + 0x2000;
    func_800A8AA8(0x1133);
    func_800A8AF0(0x1134);
    func_800A8B38(0x1135);
    cdrom_queue_read(0x1136, D_8011D538);
    cdrom_queue_read(0x1137, D_8011F538);
    cdrom_queue_read(0x1138, D_800DCF18);
    cdrom_queue_read(0x1139, D_8011CF1C);
    cdrom_queue_read(0x113A, D_8011CF28);
    cdrom_queue_read(0x113B, D_8011CF2C);
    cdrom_queue_read(0x113C, D_8011CF30);
}
