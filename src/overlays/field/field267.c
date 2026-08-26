#include "common.h"

extern void (*D_800F19D8[])(s32 arg0);

void func_800C5704(s32 arg0)
{
    if (arg0 < 0x60)
    {
        D_800F19D8[arg0](arg0);
        return;
    }
    akao_set_song_params(0x8002, arg0, 0, 0);
}
