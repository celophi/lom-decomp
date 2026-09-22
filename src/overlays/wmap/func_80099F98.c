/* Partial WMAP decompilation: 99.894740% (gcc280_g0). */
#include "common.h"

extern s16 D_800D6508[];
extern s32 D_800D9164;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_8019D240;
extern s16 D_801AFBD2;
extern s32 func_80099E84(void);
extern void func_80099754(s32);
extern void func_8006CBD8(s32 (*)(void));

/** @brief Wait for facing to settle, then select the next movement direction.
 * @return One while waiting, zero after scheduling movement.
 */
s32 func_80099F98(void)

{
    s32 delta_y;
    s32 delta_x;
    s32 magnitude_x;
    s32 magnitude_y;

    if (D_800D9164 == 0)
   
  {
        func_80099754(1);
        if (D_801AFBD2 == D_800D6508[D_8019D240])
       
      {
            D_801398D0 = 2;
            D_800D9164++;
        }
        return 1;
    }
    if (D_801398D0 == 2)
   
  {
        return 1;
    }
    D_800D9164 = 0;
    delta_x = D_800DCED8 - D_800DCEF8;
    delta_y = D_800DCEE4 - D_800DCF00;
    if (delta_x != 0)
    {
        magnitude_x = delta_x;
        if (delta_x < 0)
        {
            magnitude_x = -magnitude_x;
        }
        delta_x = (delta_x / magnitude_x) + 1;
    }
    else
    {
        delta_x = 1;
    }
    if (delta_y != 0)
    {
        magnitude_y = delta_y;
        if (delta_y < 0)
        {
            magnitude_y = -magnitude_y;
        }
        delta_y = (delta_y / magnitude_y) + 1;
    }
    else
    {
        delta_y = 1;
    }
    D_8019D240 = delta_x + (delta_y * 3);
    func_8006CBD8(func_80099E84);
    return 0;
}
