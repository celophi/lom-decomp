#include "common.h"
#include "sdk/libgte.h"

extern SVECTOR D_80139278;
extern VECTOR D_8011CF60;

/**
 * @brief Compose the global and local transforms and install the result in the GTE.
 * @param translation Translation for the global rotation matrix.
 * @param rotation Local rotation angles.
 */
void func_8006D1AC(VECTOR *translation, SVECTOR *rotation)
{
    MATRIX matrices[2];

    RotMatrix(&D_80139278, &matrices[0]);
    TransMatrix(&matrices[0], translation);
    SetRotMatrix(&matrices[0]);
    SetTransMatrix(&matrices[0]);
    RotMatrix(rotation, &matrices[1]);
    TransMatrix(&matrices[1], &D_8011CF60);
    CompMatrix(&matrices[0], &matrices[1], &matrices[1]);
    SetRotMatrix(&matrices[1]);
    SetTransMatrix(&matrices[1]);
}
