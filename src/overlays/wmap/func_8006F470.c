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
extern WmapOrientation D_801B24A8;
extern WmapTransform D_80182DC0;
extern WmapTransform D_801B2478;
extern s32 D_801B2470;
extern s32 D_801B2428;
extern s32 D_801B242C;
extern void func_8006E3E4(void);

/** @brief Restore the default transform and advance the sequence. */
void func_8006F470(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_801B2470 = 0x80;
    D_801B242C = 1;
    D_801B2428++;
    func_8006E3E4();
}
