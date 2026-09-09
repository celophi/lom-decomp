typedef int s32;
typedef unsigned int u32;
typedef short s16;
typedef unsigned char u8;
/** Nine-entry relationship lookup copied into the calculation workspace. */
typedef struct Lookup
{
    s32 values[9];
} Lookup;
extern Lookup D_80051F04;
extern u8 D_80122C06;
extern s16 D_80122C10[2];
/**
 * Apply packed category and element multipliers to the two pending values.
 *
 * Both results remain signed 32-bit values until clamped to 0..32767.
 * @note Partial match; compiler and expression probes are retained in working/func_800C9960.
 */
void func_800C9960(void)
{
    Lookup lookup;
    u8 *base;
    s32 packed;
    s32 first;
    s32 second;
    s32 element;
    s32 amount;
    s32 level;
    s32 primary;
    s32 secondary;
    s32 third;
    u32 fourth;
    s32 pair;
    s32 low;
    u32 high;
    s32 selected;
    s32 mode;
    s32 factor1;
    s32 factor2;
    s32 element1;
    s32 element2;
    s32 product1, product2, scaled1, scaled2;
    s32 clamp;
    s32 result1;
    s32 result2;
    lookup = D_80051F04;
    base = &D_80122C06;
    packed = base[0];
    first = *(s16 *)(base + 10);
    second = *(s16 *)(base + 12);
    element = base[-3];
    amount = base[-2];
    level = base[2];
    primary = packed & 3;
    secondary = (packed >> 2) & 3;
    third = (packed >> 4) & 3;
    fourth = (u8)packed >> 6;
    packed = base[-1];
    pair = packed;
    low = pair & 15;
    high = (u8)pair >> 4;
    packed = base[1];
    selected = packed;
    mode = base[4];
    factor1 = 10;
    if (selected != primary)
    {
        factor1 = 2;
        if (selected == secondary)
        {
            factor1 = 1;
        }
    }
    factor2 = 10;
    if (selected != third)
    {
        factor2 = 2;
        if (selected == fourth)
        {
            factor2 = 1;
        }
    }
    element1 = 2;
    if (mode == 0)
    {
        if (element == low)
        {
            element1 = 3;
        }
        else
        {
            element1 = 2;
            if (element == lookup.values[low])
            {
                element1 = 1;
            }
        }
        if (element == high)
        {
            element2 = 3;
        }
        else
        {
            element2 = 2;
            if (element == lookup.values[high])
            {
                element2 = 1;
            }
        }
    }
    else
    {
        element2 = 2;
    }
    first /= level + 6;
    second /= level + 6;
    product1 = factor1 * element1;
    product2 = factor2 * element2;
    scaled1 = product1 * amount;
    scaled2 = product2 * amount;
    first += scaled1;
    second += scaled2;
    first *= level + 7;
    second *= level + 7;
    clamp = 0x7FFF;
    if (first >= 0)
    {
        if (first <= 0x7FFF)
        {
            clamp = first;
        }
    }
    else
    {
        clamp = 0;
    }
    first = clamp;
    if (second >= 0)
    {
        clamp = 0x7FFF;
        if (second <= 0x7FFF)
        {
            clamp = second;
        }
    }
    else
    {
        clamp = 0;
    }
    D_80122C10[0] = first;
    D_80122C10[1] = clamp;
}
