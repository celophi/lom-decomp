#include "common.h"

typedef struct {
    s32 values[13];
} ZukanS32x13;

typedef struct {
    s32 values[7];
} ZukanS32x7;

typedef struct {
    s16 values[1018];
} ZukanS16x1018;

typedef struct {
    u16 values[719];
} ZukanU16x719;

extern ZukanS32x13 D_8014001C;
extern ZukanS32x7 D_80140050;
extern ZukanS16x1018 D_8014006C;
extern ZukanU16x719 D_80140860;

extern u32 D_8004300C[];
extern u32 D_80043038[];
extern u8 D_800432C8[];
extern s32 D_80043454;
extern u32 D_800460AC[];
extern u32 D_800460EC;
extern u32 D_800460FC;
extern u32 D_80046124;
extern u32 D_80046128;

/**
 * @brief Build and filter the ZUKAN entry list for the requested category.
 * @param arg0 Category index to process.
 * @param arg1 Output array of entry values.
 * @param arg2 Output array of entry indices.
 * @return Number of entries remaining after filtering.
 */
s32 func_80142D08(s32 arg0, s32* arg1, s32* arg2)
{
    s32 count;
    s32 i;
    s32 j;
    s32 unused;
    s32 swap;
    s32 selected_count;
    s32 range_offsets[13];
    s32 group_offsets[7];
    s16 values[1018];
    u16 order[719];

    __builtin_memcpy(range_offsets, range_offsets, 0);
    *(ZukanS32x13*)range_offsets = D_8014001C;
    __builtin_memcpy(group_offsets, group_offsets, 0);
    *(ZukanS32x7*)group_offsets = D_80140050;
    __builtin_memcpy(values, values, 0);
    *(ZukanS16x1018*)values = D_8014006C;
    __builtin_memcpy(order, order, 0);
    *(ZukanU16x719*)order = D_80140860;

    if (arg0 == 8)
    {
        range_offsets[arg0 + 1] = 0x240;
    }
    if (arg0 == 7)
    {
        range_offsets[arg0 + 1] = 0x1C6;
    }

    count = range_offsets[arg0 + 1] - range_offsets[arg0];
    i = 0;
    while (i < count)
    {
        arg2[i] = i + range_offsets[arg0];
        i++;
    }

    if (arg0 == 0xB)
    {
        i = count;
        while (i < count + 0x1A)
        {
            arg2[i] = i - ({ count - 0x1C6; });
            i++;
        }
        count += 0x1A;
    }

    selected_count = 0;
    i = 0;
    while (order[i] != 0x400)
    {
        j = selected_count;
        while (j < count)
        {
            if (order[i] == arg2[j])
            {
                swap = arg2[selected_count];
                arg2[selected_count] = arg2[j];
                arg2[j] = swap;
                arg1[selected_count] = values[arg2[selected_count]];
                selected_count++;
            }
            j++;
        }
        i++;
    }
    count = selected_count;

    if (arg0 == 1)
    {
        i = 0;
        while (i < 0x21)
        {
            if (((D_800432C8[i * 12] >> 1) & 1) & 0xFF)
            {
                D_800460AC[(i + 0x200) / 32] = D_800460AC[(i + 0x200) / 32] | (1 << ((i + 0x200) % 32));
            }
            i++;
        }
        if (D_80043454 & 4)
        {
            D_800460EC |= 0x01000000;
        }
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(arg2[i] + 0x1FF) / 32] & (1 << ((arg2[i] + 0x1FF) % 32))))
            {
                arg1[i] = 0;
                arg2[i] = 0;
            }
            i++;
        }
    }
    else if (arg0 == 2)
    {
        i = 0;
        while (i < 0x21)
        {
            if ((D_800432C8[i * 12] & 1) & 0xFF)
            {
                D_800460AC[(i + 0x240) / 32] = D_800460AC[(i + 0x240) / 32] | (1 << ((i + 0x240) % 32));
            }
            if (((D_800432C8[i * 12] >> 1) & 1) & 0xFF)
            {
                D_800460AC[(i + 0x280) / 32] = D_800460AC[(i + 0x280) / 32] | (1 << ((i + 0x280) % 32));
            }
            i++;
        }
        if (D_80043454 & 4)
        {
            D_800460FC |= 0x01000000;
        }
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(arg2[i] + 0x1FF) / 32] & (1 << ((arg2[i] + 0x1FF) % 32))))
            {
                if (!(D_800460AC[(arg2[i] + 0x23F) / 32] & (1 << ((arg2[i] + 0x23F) % 32))))
                {
                    arg1[i] = 0;
                    arg2[i] = 0;
                }
            }
            if (D_800460AC[(arg2[i] + 0x23F) / 32] & (1 << ((arg2[i] + 0x23F) % 32)))
            {
                arg1[i] = values[arg2[i] + 0x1FF];
            }
            i++;
        }
    }
    else if (arg0 == 3)
    {
    }
    else if (arg0 == 4)
    {
    }
    else if (arg0 == 5)
    {
    }
    else if (arg0 == 6)
    {
        i = 0;
        while (i < count)
        {
            if (arg2[i] != 0x135)
            {
                if (arg2[i] < 0x136)
                {
                    if (!(D_800460AC[(arg2[i] - 0xE6) / 32] & (1 << ((arg2[i] - 0xE6) % 32))))
                    {
                        arg1[i] = 0;
                        arg2[i] = 0;
                    }
                }
                else
                {
                    if (!(D_800460AC[(arg2[i] - 0xA) / 32] & (1 << ((arg2[i] - 0xA) % 32))))
                    {
                        arg1[i] = 0;
                        arg2[i] = 0;
                    }
                }
            }
            i++;
        }
    }
    else if (arg0 == 7)
    {
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(arg2[i] - 0xFA) / 32] & (1 << ((arg2[i] - 0xFA) % 32))))
            {
                arg1[i] = 0;
                arg2[i] = 0;
            }
            i++;
        }
    }
    else if (arg0 == 8)
    {
        j = 0;
        while (j < 6)
        {
            if (!(D_800460AC[(j + 0x122) / 32] & (1 << ((j + 0x122) % 32))))
            {
                i = 0;
                while (i < count)
                {
                    if ((arg2[i] >= group_offsets[j] + 0x1E0) && (arg2[i] < group_offsets[j + 1] + 0x1E0))
                    {
                        arg1[i] = 0;
                        arg2[i] = 0;
                    }
                    i++;
                }
            }
            j++;
        }
    }
    else if (arg0 == 9)
    {
        i = 0;
        while (i < count)
        {
            if (!(D_800460AC[(arg2[i] - 0x1B8) / 32] & (1 << ((arg2[i] - 0x1B8) % 32))))
            {
                arg1[i] = 0;
                arg2[i] = 0;
            }
            i++;
        }
    }
    else if (arg0 == 0xA)
    {
    }
    else if (arg0 == 0xB)
    {
        i = 0;
        while (i < 0x60)
        {
            if ((D_80043038[i / 32] >> (i % 32)) & 1)
            {
                if (i == 8)
                {
                    D_80046124 |= 0x100;
                }
                if (i == 0xA)
                {
                    D_80046124 |= 0x200;
                }
                if ((i >= 0x2F) && (i < 0x46))
                {
                    D_800460AC[(i + 0x39B) / 32] = D_800460AC[(i + 0x39B) / 32] | (1 << ((i + 0x39B) % 32));
                }
                if (i == 0x4F)
                {
                    D_80046128 |= 2;
                }
            }
            i++;
        }

        i = 0;
        while (i < 0x160)
        {
            if ((i % 32) < 0x18)
            {
                if ((D_8004300C[i / 32] >> (i % 32)) & 1)
                {
                    j = i - ((i / 32) * 8);
                    D_800460AC[(j + 0x2C0) / 32] = D_800460AC[(j + 0x2C0) / 32] | (1 << ((j + 0x2C0) % 32));
                }
            }
            i++;
        }

        i = 0;
        while (i < count)
        {
            if ((arg2[i] >= 0x1C6) && (arg2[i] < 0x2EE))
            {
                if (!(D_800460AC[(arg2[i] + 0x202) / 32] & (1 << ((arg2[i] + 0x202) % 32))))
                {
                    arg1[i] = 0;
                    arg2[i] = 0;
                }
            }
            else
            {
                if (!(D_800460AC[(arg2[i] - 0x2E) / 32] & (1 << ((arg2[i] - 0x2E) % 32))))
                {
                    arg1[i] = 0;
                    arg2[i] = 0;
                }
            }
            i++;
        }
    }

    return count;
}
