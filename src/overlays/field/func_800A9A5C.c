#include "common.h"

extern s32 D_801227F8[];
extern s32 D_80122908;
extern u8 D_80122910[];

/**
 * @brief Merge duplicate entries in the field word/amount list and compact removed slots.
 */
void func_800A9A5C(void)
{
    s32 i;
    s32 j;
    u8 amount;
    s32 k;

    i = 0;
    if (D_80122908 > 0)
    {
        do
        {
            if (D_80122910[i] != 0)
            {
                for (j = 0; j < i; j++)
                {
                    amount = D_80122910[j];
                    if ((amount != 0) && (D_801227F8[i] == D_801227F8[j]))
                    {
                        k = i;
                        D_80122910[j] = amount + D_80122910[i];
                        while (k < (D_80122908 - 1))
                        {
                            D_80122910[k] = D_80122910[k + 1];
                            D_801227F8[k] = D_801227F8[k + 1];
                            k += 1;
                        }
                        i -= 1;
                        D_80122908 -= 1;
                        break;
                    }
                }
            }
            i += 1;
        } while (i < D_80122908);
    }
}
