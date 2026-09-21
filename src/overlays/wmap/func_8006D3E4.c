#include "common.h"

extern s32 D_8011CF44;
extern s32 D_801B10A8;
extern void func_8006D420(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8006D3E4(void)
{
    if (D_8011CF44 == 0)
    {
        D_801B10A8 += 1;
        func_8006D420();
    }
}
