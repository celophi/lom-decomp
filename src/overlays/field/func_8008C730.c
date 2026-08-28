#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[0x21 - 0xC];
    u8 unk21;
} FieldPositionRecord;

typedef struct
{
    s16 x;
    s16 y;
} FieldPosition16;

extern FieldPositionRecord D_800FDF58;
extern FieldPosition16 D_80105B4C[];
extern u8 D_8010CFE0[];

/**
 * @brief Copies the active field position and status into 48 compact entries.
 *
 * The fixed-point coordinates are divided by 256 with truncation toward zero
 * before being stored as signed halfwords.
 */
void func_8008C730(void)
{
    FieldPosition16 *dest;
    s32 index;
    s32 value;

    dest = D_80105B4C;
    index = 0;
    do
    {
        value = D_800FDF58.unk0;
        if (value < 0)
        {
            value += 0xFF;
        }
        dest->x = value >> 8;

        value = D_800FDF58.unk8;
        if (value < 0)
        {
            value += 0xFF;
        }
        dest->y = value >> 8;
        D_8010CFE0[index] = D_800FDF58.unk21;
        index++;
        dest++;
    } while (index < 0x30);
}
