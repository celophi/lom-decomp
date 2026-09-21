#include "common.h"

#include "sdk/libgte.h"

extern VECTOR D_8011CF60;

/**
 * @brief Install a rotation matrix with the world-map translation.
 * @param rotation Rotation angles used to build the matrix.
 */
void func_8006D150(SVECTOR *rotation)
{
    MATRIX matrix;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}
