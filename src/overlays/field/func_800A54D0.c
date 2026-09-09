#include "common.h"
#include "sdk/libgpu.h"

extern u8 *g_pad_ctx;
extern u8 D_800EDED8[];

/** @brief Active flag and palette index in the runtime slot context. */
typedef struct
{
    u8 pad0[0x2B0C];
    u8 unk2B0C;
    u8 pad2B0D[0x2B54 - 0x2B0D];
    s32 unk2B54;
} PadCtxB800A54D0;

/** @brief Upload palettes for the three active runtime slots. */
void func_800A54D0(void)
{
    RECT rect;
    PadCtxB800A54D0 *p;
    s32 i;
    s32 offset;

    i = 0;
    offset = i;
    do
    {
        p = (PadCtxB800A54D0 *) (g_pad_ctx + offset);
        if (p->unk2B0C != 0)
        {
            if ((u32) p->unk2B54 < 0x10)
            {
                rect.x = 0x100;
                rect.y = i + 0x1F0;
                rect.w = 0x10;
                rect.h = 1;
                LoadImage(&rect, (u_long *)(D_800EDED8 + (p->unk2B54 << 5)));
            }
            else
            {
                rect.x = 0x100;
                rect.y = i + 0x1F0;
                rect.w = 0x10;
                rect.h = 1;
                LoadImage(&rect, (u_long *)((D_800EDED8 + 0x200) + ((p->unk2B54 - 0x10) << 5)));
            }
        }
        i += 1;
        offset += 0x14C;
    } while (i < 3);
}
