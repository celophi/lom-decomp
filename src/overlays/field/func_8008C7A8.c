#include "common.h"

/**
 * @brief Sparse view of the actor fields used to refresh position history.
 */
typedef struct
{
    u8 pad0[0x54];
    s32 primary_x;
    u8 pad58[4];
    s32 primary_z;
    u8 pad60[0x10];
    s32 primary_flags;
    u8 pad74[5];
    u8 primary_actor;
    u8 pad7a[0xA8 - 0x7A];
    s32 secondary_x;
    u8 pad_ac[4];
    s32 secondary_z;
    u8 pad_b4[0xCD - 0xB4];
    u8 secondary_actor;
} FieldActors;

/**
 * @brief Sparse position-history view used at the base and indexed offsets.
 */
typedef struct
{
    u8 pad0[0x6C];
    s16 x;
    s16 z;
    u8 pad70[0x3AA - 0x70];
    u8 primary_index;
    u8 pad3ab[0x5E6 - 0x3AB];
    u8 secondary_index;
} FieldHistory;

void func_8008CAA4(void *, s32, void *, void *, s32, s32);
extern FieldActors D_800FDF58;
extern u8 D_800FDFAC[];
extern u8 D_800FE000[];
extern FieldHistory D_80105AE0;
extern u8 D_80105B4C[];

/**
 * @brief Refresh actor position histories when their current samples differ.
 * @note Coordinate adjustment before shifting preserves truncation toward zero.
 * @note The primary path requires an active actor and a nonzero low flag field.
 * @note GCC 2.7.2 CDK matches all 191 instructions (764 bytes).
 */
void func_8008C7A8(void)
{
    s32 primary_x;
    s32 secondary_x;
    s32 primary_only_x;
    s32 secondary_only_x;
    s32 primary_z;
    s32 secondary_z;
    s32 primary_only_z;
    s32 secondary_only_z;
    FieldHistory *primary_history;
    FieldHistory *secondary_history;
    FieldHistory *primary_only_history;
    FieldHistory *secondary_only_history;
    FieldHistory *history_base;

    if ((D_800FDF58.primary_actor != 0xFF) && (D_800FDF58.primary_flags & 0x1FF))
    {
        if (D_800FDF58.secondary_actor != 0xFF)
        {
            primary_x = D_800FDF58.primary_x;
            if (primary_x < 0)
            {
                primary_x += 0xFF;
            }
            primary_history = (FieldHistory *)((D_80105AE0.primary_index * 4) + (u8 *)&D_80105AE0);
            if ((primary_x >> 8) == primary_history->x)
            {
                primary_z = D_800FDF58.primary_z;
                if (primary_z < 0)
                {
                    primary_z += 0xFF;
                }
                if ((primary_z >> 8) == primary_history->z)
                {
                    secondary_x = D_800FDF58.secondary_x;
                    if (secondary_x < 0)
                    {
                        secondary_x += 0xFF;
                    }
                    secondary_history = (FieldHistory *)((D_80105AE0.secondary_index * 4) + (u8 *)&D_80105AE0);
                    if ((secondary_x >> 8) == secondary_history->x)
                    {
                        secondary_z = D_800FDF58.secondary_z;
                        if (secondary_z < 0)
                        {
                            secondary_z += 0xFF;
                        }
                        if ((secondary_z >> 8) != secondary_history->z)
                        {
                            goto refresh_both;
                        }
                    }
                    else
                    {
                        goto refresh_both;
                    }
                }
                else
                {
                    goto refresh_both;
                }
            }
            else
            {
refresh_both:
                func_8008CAA4(D_80105B4C, 0x18, D_800FDFAC, D_800FDFAC, 0, 0x18);
                func_8008CAA4(D_80105B4C + 0x60, 0x17, D_800FDFAC, D_800FDFAC - 0x54, 0x18, 0x18);
                history_base = (FieldHistory *)(D_80105B4C - 0x6C);
                history_base->primary_index = 0x18;
                func_8008CAA4(D_80105B4C, 0x18, D_800FDFAC + 0x54, D_800FDFAC, 0, 0x18);
                history_base->secondary_index = 0;
            }
        }
        else
        {
            primary_only_x = D_800FDF58.primary_x;
            if (primary_only_x < 0)
            {
                primary_only_x += 0xFF;
            }
            primary_only_history = (FieldHistory *)((D_80105AE0.primary_index * 4) + (u8 *)&D_80105AE0);
            if ((primary_only_x >> 8) == primary_only_history->x)
            {
                primary_only_z = D_800FDF58.primary_z;
                if (primary_only_z < 0)
                {
                    primary_only_z += 0xFF;
                }
                if ((primary_only_z >> 8) != primary_only_history->z)
                {
                    goto refresh_primary;
                }
            }
            else
            {
refresh_primary:
                func_8008CAA4(D_80105B4C, 0x18, D_800FDFAC, D_800FDFAC, 0, 0x18);
                func_8008CAA4(D_80105B4C + 0x60, 0x17, D_800FDFAC, D_800FDFAC - 0x54, 0x18, 0x18);
                D_80105B4C[0x33E] = 0x18;
            }
        }
    }
    else if (D_800FDF58.secondary_actor != 0xFF)
    {
        secondary_only_x = D_800FDF58.secondary_x;
        if (secondary_only_x < 0)
        {
            secondary_only_x += 0xFF;
        }
        secondary_only_history = (FieldHistory *)((D_80105AE0.secondary_index * 4) + (u8 *)&D_80105AE0);
        if ((secondary_only_x >> 8) == secondary_only_history->x)
        {
            secondary_only_z = D_800FDF58.secondary_z;
            if (secondary_only_z < 0)
            {
                secondary_only_z += 0xFF;
            }
            if ((secondary_only_z >> 8) != secondary_only_history->z)
            {
                goto refresh_secondary;
            }
        }
        else
        {
refresh_secondary:
            func_8008CAA4(D_80105B4C, 0x18, D_800FE000, D_800FE000, 0, 0x18);
            func_8008CAA4(D_80105B4C + 0x60, 0x17, D_800FE000, D_800FE000 - 0xA8, 0x18, 0x18);
            D_80105B4C[0x57A] = 0x18;
        }
    }
}
