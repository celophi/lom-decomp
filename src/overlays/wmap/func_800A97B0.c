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
void func_800A97B0(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = (u8 *)D_8011CF1C + 0x2000;
    func_800A8AA8(0x117F);
    func_800A8AF0(0x1180);
    func_800A8B38(0x1181);
    cdrom_queue_read(0x1182, D_800DCF18);
    cdrom_queue_read(0x1183, D_8011CF1C);
    cdrom_queue_read(0x1184, D_8011D538);
    cdrom_queue_read(0x1185, D_8011F538);
    cdrom_queue_read(0x1186, D_80121538);
}
