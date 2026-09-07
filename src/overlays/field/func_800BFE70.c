#include "common.h"

typedef struct
{
    u8 pad0[4];
    u16 unk4;
} TableEntryB800BFE70;

u8* func_800C1E40(s32 arg0);

/**
 * @brief Copy two encoded text entries into a destination buffer.
 * @param arg0 Index of the second source entry.
 * @param arg1 Index of the first source entry.
 * @param arg2 Destination buffer.
 */
void func_800BFE70(s32 arg0, s32 arg1, u8* arg2)
{
    u8* base;
    u8* src;
    u8* dst;
    s32 c2;

    dst = arg2;
    base = func_800C1E40(8);

    src = base + (((TableEntryB800BFE70*)(base + (arg1 << 1)))->unk4 + 4);
    if (*src != 0)
    {
        s32 c1;

        do
        {
            c1 = *src;
            if (c1 < 0x20)
            {
                if (c1 >= 0x1D)
                {
                    *dst = c1;
                    src += 1;
                    dst += 1;
                }
            }
            c2 = *src;
            src += 1;
            *dst = c2;
            dst += 1;
        } while (*src != 0);
    }

    src = base + (((TableEntryB800BFE70*)(base + (arg0 << 1)))->unk4 + 4);
    if (*src != 0)
    {
        s32 c1;

        do
        {
            c1 = *src;
            if (c1 < 0x20)
            {
                if (c1 >= 0x1D)
                {
                    *dst = c1;
                    src += 1;
                    dst += 1;
                }
            }
            c2 = *src;
            src += 1;
            *dst = c2;
            dst += 1;
        } while (*src != 0);
    }

    *dst = 0;
}
