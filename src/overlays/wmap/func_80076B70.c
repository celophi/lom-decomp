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
extern WmapOrientation D_801B24A0;
extern WmapTransform D_80182DC0;
extern WmapTransform D_80139888;
extern s32 D_801B25D8;
extern s32 D_801B25D0;
extern s32 D_801B25D4;
extern void func_80074C2C(void);

/** @brief Restore the default transform and advance the sequence. */
void func_80076B70(void)
{
    D_801B25D8 = 0;
    D_801B24A0 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80139888.words[2] = 0xC350;
    D_801B25D4 = 10;
    D_801B25D0++;
    func_80074C2C();
}
