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
extern s32 D_801B24B4;
extern s32 D_801B2608;
extern s32 D_801B260C;
extern void func_80076EB4(void);

/** @brief Restore the default transform and advance the sequence. */
void func_80077F14(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_801B24B4 = 0x80;
    D_80139870.words[2] = 1000;
    D_801B260C = 0x80;
    D_801B2608++;
    func_80076EB4();
}
