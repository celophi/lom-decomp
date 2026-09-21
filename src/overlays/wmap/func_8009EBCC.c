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
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

/** @brief Set the selected record value and advance to a 50-tick delay. */
void func_8009EBCC(void)
{
    D_801B2CDC = 50;
    D_80139290[D_8011D510][D_8011D530].value = 0x117;
    D_801B2CD8 += 1;
}
