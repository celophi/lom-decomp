#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800E7F64[];
extern u8 D_800EA560[];
extern u8 D_800EAA54[];
extern u8 D_800EBED8[];
extern u8 D_800ECEB4[];
extern u8 D_800ED310[];
extern u8 D_800EE2EC[];
extern u8 D_800F2238[];
extern u8 D_800F3214[];
extern u8 D_800FAE60[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern u8 *D_8011CF30;
extern u8 *D_8011CF34;
extern u8 *D_8011CF38;
extern u8 *D_8011CF3C;
extern u8 *D_8011CF40;
extern u8 *D_8011CF84;
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];

/** @brief Queue the world-map effect resource set. */
void func_800AA564(void)
{
    D_8011CF1C = D_800E7F64;
    D_8011CF24 = D_800EA560;
    D_8011CF28 = D_800EAA54;
    D_8011CF2C = D_800EBED8;
    D_8011CF30 = D_800ECEB4;
    D_8011CF34 = D_800ED310;
    D_8011CF38 = D_800EE2EC;
    D_8011CF3C = D_800F2238;
    D_8011CF40 = D_800F3214;
    D_8011CF84 = D_800FAE60;
    func_800A8AA8(0x122C);
    func_800A8AF0(0x122D);
    func_800A8B38(0x122E);
    cdrom_queue_read(0x122F, D_8011D538);
    cdrom_queue_read(0x1230, D_8011F538);
    cdrom_queue_read(0x1231, D_80121538);
    cdrom_queue_read(0x1232, D_800DCF18);
}
