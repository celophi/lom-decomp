#include "common.h"

extern u8 *g_pad_ctx;

/**
 * @brief Find the first free inventory record in g_pad_ctx's item table.
 * @return Pointer to the first record whose first byte is 0, or NULL if all
 *         0x64 records are occupied.
 */
u8 *func_800A9060(void)
{
    s32 count;
    u8* rec;

    rec = g_pad_ctx + 0xCE0;
    for (count = 0; count < 0x64; count++)
    {
        if (*rec == 0)
        {
            return rec;
        }
        rec += 0x40;
    }
    return NULL;
}

extern void field_text_reset_scratch(void);
extern void func_80063194(void);
extern void func_800A92CC(void *arg0);
extern void func_800A939C(void *arg0);
extern s32 D_80122984;

void func_800A909C(void *arg0)
{
    field_text_reset_scratch();

    if (D_80122984)
    {
        func_800A92CC(arg0);
    }
    else
    {
        func_800A939C(arg0);
    }

    func_80063194();
}
