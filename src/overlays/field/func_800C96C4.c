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
extern Layout g_menuLayoutBuffer;
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
    layout = &g_menuLayoutBuffer;
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
