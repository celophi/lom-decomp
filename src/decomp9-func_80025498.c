#include "akao.h"
#include "akao_driver.h"

typedef struct
{
    u8 pad0[4];
    s16 value;
    u16 pad6;
} AkaoVoiceEntry;

extern AkaoVoiceEntry D_8004C1A0[];

s32 func_80025498(s32 arg0)
{
    AkaoVoiceEntry* entry;

    if (arg0 != 0)
    {
        arg0 = 0;
    }
    else
    {
        arg0 = g_akao_seq_channel0->voice_alloc_base;
    }

    entry = &D_8004C1A0[arg0];
    if (entry->value != 0)
    {
        arg0++;
        while (1)
        {
            if (arg0 >= 0x18)
            {
                break;
            }

            entry++;
            arg0++;
            if (entry->value != 0)
            {
                continue;
            }

            arg0--;
            break;
        }
    }

    return arg0;
}