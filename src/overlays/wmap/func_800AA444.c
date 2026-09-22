#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DDEF4[];
extern u8 D_800DEED0[];
extern u8 D_800DFEAC[];
extern u8 D_800E0720[];
extern u8 D_800E0F94[];
extern u8 D_800E91C0[];
extern u8 D_800F420C[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 D_80123538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern u8 *D_8011CF30;
extern u8 *D_8011CF34;
extern u8 *D_8011CF38;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800AA444(void)
{
    D_8011CF1C = D_800DDEF4;
    D_8011CF24 = D_800DEED0;
    D_8011CF28 = D_800DFEAC;
    D_8011CF2C = D_800E0720;
    D_8011CF30 = D_800E0F94;
    D_8011CF34 = D_800E91C0;
    D_8011CF38 = D_800F420C;
    func_800A8AA8(0x120E);
    func_800A8AF0(0x120F);
    func_800A8B38(0x1210);
    cdrom_queue_read(0x1211, D_8011D538);
    cdrom_queue_read(0x1212, D_8011F538);
    cdrom_queue_read(0x1213, D_80121538);
    cdrom_queue_read(0x1214, D_80123538);
    cdrom_queue_read(0x1215, D_800DCF18);
}
