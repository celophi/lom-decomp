#include "common.h"

extern u8 D_800DEF18;
extern u8 D_800DCF18;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern void func_800A8AA8(s32 id);
extern void func_800A8AF0(s32 id);
extern void func_800A8B38(s32 id);
extern void cdrom_queue_read(s32 id, void *buf);

/** @brief World-map load: register buffers and queue CD reads for a set. */
void func_800A9960(void)
{
    D_8011CF1C = &D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x118F);
    func_800A8AF0(0x1190);
    func_800A8B38(0x1191);
    cdrom_queue_read(0x1192, &D_8011D538);
    cdrom_queue_read(0x1193, &D_8011F538);
    cdrom_queue_read(0x1194, &D_80121538);
    cdrom_queue_read(0x1195, &D_800DCF18);
    cdrom_queue_read(0x1196, D_8011CF1C);
}
