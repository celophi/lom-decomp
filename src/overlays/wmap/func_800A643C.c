#include "wmap_sequence_runtime.h"
#include "common.h"

/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern WmapTransform D_800DCEC8;
extern WmapTransform D_80139950;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_8011CF50;
extern s32 D_80139234;
extern s32 D_801398D0;
extern s32 D_8013B208;
extern s32 D_8013B288;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E40;
extern void func_800A76F8(void);

/** @brief Save the projection state and set the next effect's map-relative position. */
void func_800A643C(void)
{
    D_8013B288 = 0;
    D_80139234 = 0;
    D_8011CF50 = 1;
    D_8013B208 = 1;
    D_800DCEC8 = D_80139950;
    func_8006D0F0(4, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 48) - D_80139950.x;
    D_80182D78 = ((D_800DCF00 - 1) * 48) - D_80139950.y;
    D_801B2E40++;
    func_800A76F8();
}
