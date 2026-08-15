#include "common.h"

typedef struct SeqHead {
    s32 unk0;
    u32 mask4;
    u32 unk8;
    u32 maskC;
} SeqHead;

typedef struct VoiceAllocEntry {
    s32 unk0;
    s16 value;
    s16 unk6;
} VoiceAllocEntry;

extern SeqHead* g_akao_seq_channel0;
extern SeqHead* g_akao_seq_channel1;
extern VoiceAllocEntry D_8004C1A0[];
extern u8 g_akao_seq_channels[];

void func_80025760(u8*, s32);

void func_800257E0(u32 mask)
{
    u32 used;
    u32 i;
    s32 one;
    s32 max;
    u32 m4;
    u32 mC;
    SeqHead* seq1;

    m4 = g_akao_seq_channel0->mask4;
    do {
        mC = g_akao_seq_channel0->maskC;
    } while (0);

    seq1 = g_akao_seq_channel1;

    do {
        used = (m4 & mC) | mask;
    } while (0);

    if (seq1 != 0) {
        used |= seq1->mask4 & seq1->maskC;
    }

    i = 0;
    one = 1;
    max = 0x7FFF;

    {
        VoiceAllocEntry* entry;
        s16* value;

        entry = D_8004C1A0;
        value = &entry->value;

        do {
            s32 c0 = (used & (one << i)) != 0;
            s32 c1 = (c0 != 0);
            s32 c2 = (c1 != 0);
            s32 c3 = (c2 != 0);
            s32 c4 = (c3 != 0);
            s32 c5 = (c4 != 0);
            s32 c6 = (c5 != 0);
            s32 c7 = (c6 != 0);

            if (c7) {
                *value = max;
            } else {
                func_8002611C(i, value);

                if (*value == 0) {
                    func_80025760(g_akao_seq_channels, i);
                }
            }

            i++;
            value = (s16*)((u8*)value + 8);
        } while (i < 0x18U);
    }
}
