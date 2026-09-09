#include "common.h"

/**
 * @brief Packed pending text entry with a six-bit frame countdown.
 */
typedef struct
{
    s32 unk0;
    u32 low : 17;
    u32 count : 6;
    u32 high : 9;
} UnkStruct801226A0;

/**
 * @brief FIELD context passed through to the text entry handler.
 */
typedef struct
{
    u8 pad0[0x40];
    u8 unk40;
    u8 pad41[0x40B8 - 0x41];
    s32 unk40B8;
} ArgA;

extern UnkStruct801226A0 D_801226A0[];
extern s32 D_801227C8;
extern s32 D_801227DC;

void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void func_80063194(void);
void func_800A6634(ArgA *arg0, UnkStruct801226A0 *arg1);

/**
 * @brief Process pending text entries, or clear their countdowns when disabled.
 * @param arg0 FIELD context forwarded to each active entry's handler.
 * @note The previous active count controls window cleanup on the next frame.
 * @note GCC 2.7.2 CDK matches 98.93258%; only s0/s1 allocation differs.
 */
void func_800A64D0(ArgA *arg0)
{
    s32 i;
    s32 count;

    count = 0;
    if (D_801227C8 != 0)
    {
        i = 0;
        for (; i < 3; i++)
        {
            D_801226A0[i].count = 0;
        }
        D_801227DC = 0;
        return;
    }

    for (i = 0; i < 3; i++)
    {
        if (D_801226A0[i].count)
        {
            count += 1;
        }
    }

    if (count != 0)
    {
        i = 0;
        field_text_reset_scratch();
        for (; i < 3; i++)
        {
            if (D_801226A0[i].count)
            {
                func_800A6634(arg0, &D_801226A0[i]);
                D_801226A0[i].count--;
            }
        }
        func_80063194();
    }
    else if (D_801227DC != 0)
    {
        field_text_reset_windows();
    }
    D_801227DC = count;
}
