#include "wmap_sequence_runtime.h"
#include "common.h"
#include "cdrom.h"
typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
} WmapAlignedQuad;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern u8 D_800DEF18;
extern s32* D_8011CF1C;
extern s32* D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern u8 D_8011D538;
extern s32 D_801398D0;
extern WmapAlignedQuad D_80139950;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2FE0;
extern void func_80064F64(s32);
extern void func_800B4648(void);

/** @brief Queue world-map effect resources and initialize their map position. */
void func_800B3434(void)
{
    D_8011CF1C = (s32*)&D_800DEF18;
    D_8011CF24 = (s32*)((u8*)D_8011CF1C + 0x3000);
    D_8011CF28 = (s32*)((u8*)D_8011CF24 + 0x3000);
    D_8011CF2C = (s32*)((u8*)D_8011CF28 + 0x3000);
    D_8011CF30 = (s32*)((u8*)D_8011CF2C + 0x3000);
    D_8011CF34 = (s32*)((u8*)D_8011CF30 + 0x3000);
    D_8011CF38 = (s32*)((u8*)D_8011CF34 + 0x3000);
    func_80064F64(0x1220);
    func_80064F64(0x1221);
    func_80064F64(0x1222);
    cdrom_queue_read(0x1223, &D_8011D538);
    cdrom_queue_read(0x1224, &D_800DEF18 - 0x2000);
    cdrom_queue_read(0x1225, D_8011CF1C);
    cdrom_queue_read(0x1226, D_8011CF24);
    cdrom_queue_read(0x1227, D_8011CF28);
    cdrom_queue_read(0x1228, D_8011CF2C);
    cdrom_queue_read(0x1229, D_8011CF38);
    cdrom_queue_read(0x122A, D_8011CF30);
    cdrom_queue_read(0x122B, D_8011CF34);
    func_8006D0F0(0, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    D_801B2FE0++;
    func_800B4648();
}
