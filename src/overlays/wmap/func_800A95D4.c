#include "common.h"
#include "cdrom.h"

extern u8 D_800ECF18[];
extern u8 D_800DCF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern void *D_8011CF1C;
extern void *D_8011CF24;
extern void func_800A8AA8(s32 arg0);
extern void func_800A8AF0(s32 arg0);
extern void func_800A8B38(s32 arg0);

/** @brief World-map step: set up load buffers and queue CD reads for a scene. */
void func_800A95D4(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = (u8 *)D_8011CF1C + 0x2000;
    func_800A8AA8(0x116E);
    func_800A8AF0(0x116F);
    func_800A8B38(0x1170);
    cdrom_queue_read(0x1171, D_800DCF18);
    cdrom_queue_read(0x1172, D_8011CF1C);
    cdrom_queue_read(0x1173, D_8011D538);
    cdrom_queue_read(0x1174, D_8011F538);
    cdrom_queue_read(0x1175, D_80121538);
}
