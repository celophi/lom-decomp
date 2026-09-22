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
extern WmapOrientation D_8013B238;
extern WmapTransform D_80182DC0;
extern WmapTransform D_80139870;
extern s32 D_801B246C;
extern s32 D_801B2488;
extern s32 D_801B2420;
extern s32 D_801B2424;
extern void func_8006E024(void);

/** @brief Restore the default transform and advance the sequence. */
void func_8006F2B4(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_801B2488 = 0;
    D_801B246C = 0x80;
    D_801B2424 = 4;
    D_801B2420++;
    func_8006E024();
}
