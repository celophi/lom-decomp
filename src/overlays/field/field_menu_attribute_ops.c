#include "common.h"

/** @brief Eight selection-index adjustments copied to the stack. */
typedef struct
{
    s32 entries[8];
} Choices;
/** @brief Layout fields used to weight the available choices. */
typedef struct
{
    u8 pad[0x2E6];
    u16 selected;
    u8 pad2[12];
    u8 values[1];
} Layout;
extern Choices D_80051ED8;
extern u8 g_menuLayoutBuffer[];
extern u16 g_music_track_index;
extern u8 D_80122C00, D_80122C05, D_80122C06;
extern s32 rand(void);
/** @brief Choose one of eight entries using fourth-power weights and the current mode. */
void func_800C96C4(void)
{
    s32 weights[8];
    Choices choices;
    s32 total, i, offset, value, square, weight, draw, selected;
    s32 *cursor, *base;
    Layout *layout;
    s32 track, clamped, byte_offset, random_product;
    u8 raw;
    u8 mode;
    total = 0;
    i = total;
    layout = (Layout *)g_menuLayoutBuffer;
    mode = D_80122C00;
    track = g_music_track_index;
    choices = D_80051ED8;
    offset = track * 12;
    do
    {
        raw = layout->values[i + offset];
        value = raw - 3;
        if (i == choices.entries[layout->selected & 0x7F])
        {
            value = raw - 2;
        }
        if (value >= 0)
        {
            clamped = 3;
            if (value < 4)
            {
                clamped = value;
            }
        }
        else
        {
            clamped = 0;
        }
        square = clamped * clamped;
        weight = square * square;
        byte_offset = i * 4;
        i++;
        base = weights;
        *(s32 *)((u8 *)base + byte_offset) = weight;
        total += weight;
    } while (i < 8);
    if (mode == 1)
    {
        total = 0x288;
    }
    random_product = rand() * total;
    draw = random_product >> 15;
    if (random_product < 0)
    {
        draw = (random_product + 0x7FFF) >> 15;
    }
    total = 0;
    selected = 0xFF;
    i = total;
    cursor = base;
loop:
{
    if (draw >= total && draw < total + *cursor)
    {
        goto found;
    }
    if (mode == 0)
    {
        total += *cursor;
    }
    else
    {
        total += 0x51;
    }
    i++;
    cursor++;
    if (i < 8)
    {
        goto loop;
    }
}
finish:
    if (mode == 0)
    {
        D_80122C05 = selected;
    }
    else
    {
        D_80122C06 = selected;
    }
    return;
found:
    selected = i;
    goto finish;
}

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EF8;

extern UnkStruct80051EF8 D_80051EF8;
extern void field_open_gosub_screen_sequence(UnkStruct80051EF8 *arg0);

/** @brief Open the attribute selection screen sequence. */
void func_800C9894(void)
{
    UnkStruct80051EF8 local;

    local = D_80051EF8;
    field_open_gosub_screen_sequence(&local);
}

typedef struct RecC98D4
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25;
    u8 unk26;
} RecC98D4;

typedef struct OutC98D4
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} OutC98D4;

extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values;
extern u8 D_80122C01;

/**
 * @brief Pack the current gosub result's color/attribute bytes into D_80122C01.
 *
 * If there are gosub results, reads the selected result's 0x40-byte layout record
 * (at @c g_menuLayoutBuffer + 0xCE0) for its 0x24 and 0x26 fields and a 6-bit
 * attribute from the 0xCF4 word; otherwise defaults to 0xFF. The 0x26 field is
 * clamped to 0..0x63 and all four bytes are written to @c D_80122C01.
 *
 * @note gcc280_g0, 100% match.
 */
void func_800C98D4(void)
{
    s32 result_value;
    s32 attr;
    s32 flags;
    s32 clamp_src;
    s32 out;
    RecC98D4 *rec;

    result_value = 0xFF;
    attr = 0xFF;
    flags = 0xFF;
    clamp_src = 0;
    if (g_gosub_result_count != 0)
    {
        u8 *buf = g_menuLayoutBuffer;
        u8 *base;
        u8 *recbase;
        result_value = g_gosub_result_values;
        base = buf + result_value * 0x40;
        recbase = buf + 0xCE0;
        rec = (RecC98D4 *)(recbase + result_value * 0x40);
        flags = rec->unk24;
        attr = (*(u32 *)(base + 0xCF4) >> 10) & 0x3F;
        clamp_src = rec->unk26;
    }
    if (clamp_src >= 0)
    {
        out = 0x63;
        if (clamp_src < 0x64)
        {
            out = clamp_src;
        }
    }
    else
    {
        out = 0;
    }
    ((OutC98D4 *)&D_80122C01)->unk0 = result_value;
    ((OutC98D4 *)&D_80122C01)->unk1 = attr;
    ((OutC98D4 *)&D_80122C01)->unk2 = flags;
    ((OutC98D4 *)&D_80122C01)->unk3 = out;
}

/** @brief Nine-entry relationship lookup copied into the calculation workspace. */
typedef struct Lookup
{
    s32 values[9];
} Lookup;
extern Lookup D_80051F04;
extern u8 D_80122C06;
extern s16 D_80122C10[2];
/**
 * @brief Apply packed category and element multipliers to the two pending values.
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
