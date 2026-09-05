#include "common.h"

extern s32 D_80117EE0;
extern void *D_80117EEC;
extern s32 D_8011F308;
extern void *D_8011F324;
extern s32 D_8011F328;
extern u8 D_8011F358[];

/**
 * @brief Copy a run of bytes from one buffer to another.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param count Number of bytes to copy.
 */
void func_800A4320(u8 *dst, u8 *src, s32 count)
{
    u8 temp;

    if (count > 0)
    {
        do
        {
            temp = *src;
            src += 1;
            count -= 1;
            *dst = temp;
            dst += 1;
        } while (count != 0);
    }
}

/**
 * @brief Claim the fixed buffer at 0x801DC000 (or 0x801DC800 when bit 11 of arg0 is set) if it is free.
 *
 * Records arg0 and arg1 in D_8011F328 and D_8011F324, sets D_8011F308 to 2
 * when arg0 is 0 and to 1 otherwise, and marks the buffer busy via D_80117EE0.
 *
 * @param arg0 Request word; bit 11 selects the upper buffer half.
 * @param arg1 Stored to D_8011F324.
 * @return The claimed buffer, or NULL when it is already busy.
 * @see decomp.me (100%)
 */
void *func_800A4348(s32 arg0, void *arg1)
{
    void *new_var;
    void *ptr;

    if (D_80117EE0 == 0)
    {
        if (arg0 & 0x800)
        {
            new_var = (void *)0x801DC800;
            ptr = new_var;
        }
        else
        {
            ptr = (void *)0x801DC000;
        }
        D_80117EEC = ptr;
        if (arg0 == 0)
        {
            D_8011F308 = 2;
        }
        else
        {
            D_8011F308 = 1;
        }
        D_8011F328 = arg0;
        new_var = arg1;
        D_8011F324 = new_var;
        D_80117EE0 = 1;
        return ptr;
    }
    return (void *)0;
}

/**
 * @brief Zero the 30 bytes of D_8011F358, last byte first.
 */
void func_800A43C0(void)
{
    s32 i = 0x1D;
    u8 *p = &D_8011F358[i];

    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}
