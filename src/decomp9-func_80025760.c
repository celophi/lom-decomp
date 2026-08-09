#include "common.h"

extern s32 g_akao_pending_channels;
extern s32 g_akao_seq_channel1;

void func_80025760(u8* channels, s32 voice)
{
    u32 i;
    s32 none;
    u8* pending;

    i = 0;
    none = 0x18;
    channels += 0xFC;
    do {
        if (voice == *(s32*)channels) {
            *(s32*)channels = none;
        }
        i++;
        channels += 0x118;
    } while (i < 0x20U);

    if (g_akao_seq_channel1 != 0) {
        i = 0;
        pending = (u8*)(g_akao_pending_channels + 0xFC);
        do {
            if (voice == *(s32*)pending) {
                *(s32*)pending = 0x18;
            }
            i++;
            pending += 0x118;
        } while (i < 0x20U);
    }
}