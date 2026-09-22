#include "common.h"

/** @brief Eight-byte record linking an index to its image data. */
typedef struct
{
    s32 index;
    u8 *image;
} WmapImageLink;

extern u8 D_8011D538[];
extern WmapImageLink D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_80182DF0;
extern s32 D_801B0FD0;
extern s32 D_801B2638;
extern s32 D_801B263C;
extern void func_80078678(void);

/** @brief Initialize twenty effect records and begin a 32-tick sequence step. */
void func_800785E4(void)
{
    s32 index;

    D_801B0FD0 = 20;
    D_80182DF0 = 255;
    for (index = 14; index < 34; index++)
    {
        *(s16 *)(D_801AFBD0 + index * 20) = 0;
        D_80139988[index + 14].image = D_8011D538;
    }
    D_801B263C = 32;
    D_801B2638++;
    func_80078678();
}
