#include "common.h"

#include "sdk/libgte.h"

extern void func_8006ADD0(VECTOR *, SVECTOR *);
extern void func_8006D1AC(VECTOR *, SVECTOR *);
extern s32 D_800D923C;

/**
 * @brief Select the world-map transform helper using the current mode.
 * @param translation Translation passed to the selected helper.
 * @param rotation Rotation passed to the selected helper.
 */
void func_8006CFA8(VECTOR *translation, SVECTOR *rotation)
{
    if (D_800D923C != 0)
    {
        func_8006ADD0(translation, rotation);
        return;
    }
    func_8006D1AC(translation, rotation);
}
