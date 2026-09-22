#include "wmap_sequence_runtime.h"
#include "common.h"

#include "sdk/libgte.h"

extern s32 func_80065620(s32);
extern CVECTOR D_8011D50C;
extern s32 D_80139288;

/**
 * @brief Store a drawing color and register its update callback.
 * @param color Packed color bytes.
 * @return Always one.
 */
s32 func_8006683C(s32 color)
{
    D_8011D50C = *(CVECTOR *)&color;
    D_80139288 = 1;
    func_8006CBD8(func_80065620);
    return 1;
}
