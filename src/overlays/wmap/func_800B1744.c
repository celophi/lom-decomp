#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"
#include "cdrom.h"

/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern WmapTransform D_80139950;
extern s16 D_800D926A;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern u8 D_800DEF18[];
extern u8 D_8011D538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2F90;
extern void func_800B2110(void);

/** @brief Queue effect resources and establish the map-relative effect position. */
void func_800B1744(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x7000;
    D_8011CF28 = D_8011CF24 + 0x3000;
    D_8011CF2C = D_8011CF24 + 0x7000;
    func_80064F64(0x1218);
    func_80064F64(0x1219);
    func_80064F64(0x121A);
    cdrom_queue_read(0x121B, D_8011D538);
    cdrom_queue_read(0x121C, D_800DEF18 - 0x2000);
    cdrom_queue_read(0x121D, D_8011CF1C);
    cdrom_queue_read(0x121E, D_8011CF24);
    cdrom_queue_read(0x121F, D_8011CF28);
    D_800D926A = -1;
    func_8006D0F0(0, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 48) - D_80139950.x;
    D_80182D78 = ((D_800DCF00 - 1) * 48) - D_80139950.y;
    D_801B2F90++;
    func_800B2110();
}
