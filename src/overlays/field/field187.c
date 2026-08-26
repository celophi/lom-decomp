#include "main.h"

void func_800C745C(void)
{
    PadContext* ctx = (PadContext*)g_menuLayoutBuffer;
    s32 idx = ctx->large_history_index;
    ((u8*)ctx)[0xC06] = 0;
    ctx->large_history_names[idx][0x46] = 0;
}
