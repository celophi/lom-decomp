#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DDEF4[];
extern u8 D_800DEA88[];
extern u8 D_800DFA64[];
extern u8 D_800E0114[];
extern u8 D_800E08E4[];
extern u8 D_800E0EB4[];
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
void func_800A9010(void)
{
    func_800A8AA8(0x112C);
    func_800A8AF0(0x112D);
    func_800A8B38(0x112E);
    cdrom_queue_read(0x112F, D_8011D538);
    cdrom_queue_read(0x1130, D_8011F538);
    cdrom_queue_read(0x1131, D_80121538);
    cdrom_queue_read(0x1132, D_800DCF18);
    D_8011CF1C = D_800DDEF4;
    D_8011CF24 = D_800DEA88;
    D_8011CF28 = D_800DFA64;
    D_8011CF2C = D_800E0114;
    D_8011CF30 = D_800E08E4;
    D_8011CF34 = D_800E0EB4;
}
