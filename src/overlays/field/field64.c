#include "common.h"

extern s32 D_8011F378;
extern u8 D_8011F388[];

/**
 * @brief Look up a byte from the D_8011F388 table using the D_8011F378
 *        index.
 * @return D_8011F388[D_8011F378].
 */
u8 func_800A4778(void)
{
    return D_8011F388[D_8011F378];
}
