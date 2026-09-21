#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DEF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 D_80123538[];
extern u8 D_80125538[];

/** @brief Queue the world-map effect resource set. */
void func_800AA1AC(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    func_800A8AA8(0x11ED);
    func_800A8AF0(0x11EE);
    func_800A8B38(0x11EF);
    cdrom_queue_read(0x11F0, D_8011D538);
    cdrom_queue_read(0x11F1, D_8011F538);
    cdrom_queue_read(0x11F2, D_80121538);
    cdrom_queue_read(0x11F3, D_80123538);
    cdrom_queue_read(0x11F4, D_80125538);
    cdrom_queue_read(0x11F5, D_800DCF18);
    cdrom_queue_read(0x11F6, D_8011CF1C);
    cdrom_queue_read(0x11F7, D_8011CF24);
    cdrom_queue_read(0x11F8, D_8011CF28);
}
