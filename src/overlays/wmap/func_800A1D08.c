#include "common.h"

extern s32 D_8011D510;
extern s32 D_8011D530;
/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern WmapValueRecord D_80139290[][6];
extern s32 D_8013B20C;
extern s32 D_801B2D58;

/** @brief Set the selected record value, clear the flag, and advance the sequence. */
void func_800A1D08(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = 0x110;
    D_801B2D58 += 1;
}
