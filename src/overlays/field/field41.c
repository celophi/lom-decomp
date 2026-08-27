#include "common.h"

void func_8009A364(void)
{
    func_8009A3A0();
}

extern s32 D_8010D030;

/**
 * @brief Clear the D_8010D030 global to zero.
 */
void func_8009A384(void)
{
    D_8010D030 = 0;
}

extern s32 D_8010D080;

/**
 * @brief Read the D_8010D080 global.
 * @return Current value of D_8010D080.
 */
s32 func_8009A390(void)
{
    return D_8010D080;
}

extern s32 D_8010D030;
extern s32 D_8010D040[];

s32 func_8009A3A0(s32 arg0)
{
    s32 count;

    count = D_8010D030;
    if (count < 0x10)
    {
        D_8010D040[count] = arg0;
        D_8010D030 = count + 1;
        return 0;
    }
    return 1;
}
