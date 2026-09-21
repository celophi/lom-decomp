#include "common.h"

/** @brief Two signed coordinates stored consecutively. */
typedef struct
{
    s16 x;
    s16 y;
} WmapCoordinatePair;

extern WmapCoordinatePair D_8011CF4C;

/**
 * @brief Set the world-map coordinate pair to its default position.
 */
void func_8006D190(void)
{
    D_8011CF4C.x = 0xA4;
    D_8011CF4C.y = 0x69;
}
