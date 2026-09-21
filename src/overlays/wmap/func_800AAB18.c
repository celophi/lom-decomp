#include "common.h"
#include "cdrom.h"

extern void func_8005FF88(s32);
extern void func_8006683C(s32);
extern u8 D_800DCF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 D_8011D538[];
extern s32 D_8013B208;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 D_801ADAE0;
extern s32 D_801ADAEC;
extern s32 D_801ADAF4;
extern s32 D_801B2E80;
extern s32 D_801B2E84;

/** @brief Set transition controls, queue resources, and start the loading countdown. */
void func_800AAB18(void)
{
    D_8013B208 = 1;
    func_8006683C(0x301020);
    D_801ADAF4 = 4;
    D_801ADAE0 = 1;
    D_801ADAEC = 0;
    func_8005FF88(-1);
    cdrom_wait_queue_empty();
    cdrom_queue_read(0x1205, D_80182E40);
    cdrom_queue_read(0x1206, D_8018B240);
    cdrom_queue_read(0x1207, D_80193640);
    cdrom_queue_read(0x1208, D_8011D538);
    cdrom_queue_read(0x1209, D_8011D538 + 0x2000);
    cdrom_queue_read(0x120A, D_8011D538 + 0x4000);
    cdrom_queue_read(0x120B, D_800DCF18);
    cdrom_queue_read(0x120C, D_8011CF1C);
    cdrom_queue_read(0x120D, D_8011CF24);
    D_801B2E84 = 0x1E;
    D_801B2E80 += 1;
}
