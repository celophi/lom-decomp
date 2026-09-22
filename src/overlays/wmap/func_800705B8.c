#include "common.h"

/** @brief Eight-byte orientation data with byte alignment. */
typedef struct
{
    u8 bytes[8];
} WmapOrientation;

/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 words[4];
} WmapTransform;

extern WmapOrientation D_80139258;
extern WmapOrientation D_8013B240;
extern WmapTransform D_80182DC0;
extern WmapTransform D_80139888;
extern s32 D_80182DE4;
extern s32 D_801B2460;
extern s32 D_801B2464;
extern void func_8006ECC0(void);

/** @brief Restore the default transform and advance the sequence. */
void func_800705B8(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DE4 = 0;
    D_80139888.words[2] = 0;
    D_801B2464 = 12;
    D_801B2460++;
    func_8006ECC0();
}
