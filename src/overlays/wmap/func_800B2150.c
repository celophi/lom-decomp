#include "common.h"

#include "cdrom.h"


extern void func_8006CAC0(void (*callback)(void));
extern void func_800B21DC(void);
extern s32 D_800D9228;
extern s16 D_8011CF4C[];
extern s32 D_8011D500;
extern s32 D_8013B20C;
extern s32 D_8013B258;
extern s32 D_80182E38;
extern s32 D_801B2F90;
extern void func_800B2244(void);

/** @brief Wait for CD work, initialize sequence state, and register its callback. */
void func_800B2150(void)
{
    cdrom_wait_queue_empty();
    D_8011CF4C[0] = 0x94;
    D_8011CF4C[1] = 0x31;
    D_8013B258 = 1;
    D_80182E38 = 4;
    D_8011D500 = 0x35;
    D_800D9228 = 0x35;
    func_8006CAC0(&func_800B2244);
    D_8013B20C = 1;
    D_801B2F90 += 1;
    func_800B21DC();
}
