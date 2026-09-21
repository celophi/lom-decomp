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
void func_800A9888(void)
{
    D_8011CF1C = &D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x1187);
    func_800A8AF0(0x1188);
    func_800A8B38(0x1189);
    cdrom_queue_read(0x118A, &D_8011D538);
    cdrom_queue_read(0x118B, &D_8011F538);
    cdrom_queue_read(0x118C, &D_80121538);
    cdrom_queue_read(0x118D, &D_800DCF18);
    cdrom_queue_read(0x118E, D_8011CF1C);
}
