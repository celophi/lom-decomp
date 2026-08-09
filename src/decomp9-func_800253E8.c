#include "akao.h"
#include "akao_driver.h"

typedef struct
{
    u8 pad0[4];
    s16 value;
    u16 pad6;
} AkaoVoiceEntry;

extern AkaoVoiceEntry D_8004C1A0[];
extern void func_80025760(u8* channels, s32 voice);

s32 func_800253E8(s32 arg0)
{
    s32 index;
    u16 best_value;
    s32 best_index;
    AkaoVoiceEntry* entry;

    if (arg0 != 0)
    {
        index = 0;
    }
    else
    {
        index = g_akao_seq_channel0->voice_alloc_base;
    }

    best_value = 0x7FFF;
    best_index = 0x18;
    entry = &D_8004C1A0[index];

    do
    {
        if (entry->value < (s16)best_value)
        {
            best_value = (u16)entry->value;
            best_index = index;
        }
        index++;
        entry++;
    } while (index < 0x18);

    if ((s16)best_value == 0x7FFF)
    {
        return 0x18;
    }

    func_80025760(g_akao_seq_channels, best_index);
    return best_index;
}