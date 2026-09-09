#include "common.h"

/** @brief Packed field-audio control values stored in the active layout. */
typedef union
{
    u32 flags;
    struct
    {
        u8 unk2E4;
        u8 unk2E5;
        u16 unk2E6;
    } fields;
} FieldAudioControl;

/** @brief Active layout view used to initialize field audio state. */
typedef struct
{
    u8 pad0[0xE4];
    s32 unkE4[8];
    u8 pad104[0x2E4 - 0x104];
    FieldAudioControl control;
    u8 pad2E8[0xC];
    u8 track_data[0x8C][0xC];
} FieldAudioState;

extern FieldAudioState* D_80122B74;
extern u8 D_800F0B48[];
extern u16 g_music_track_index;

s32* func_800C1EC8(s32* src, s32* dest, s32 n);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
s32 func_800C3688(s32 arg0);
s32 rand(void);

/**
 * @brief Initialize field audio variables from the active layout and music track.
 */
void func_800B0EFC(void)
{
    s32 flags;

    flags = D_80122B74->control.flags;
    if (flags & 0x800000)
    {
        D_80122B74->control.flags = flags & 0xFF7FFFFF;
        func_800C1EC8(NULL, D_80122B74->unkE4, 0x20);
        func_800BD520(0, 0xFA, rand() & 0xFF);
    }

    if (D_80122B74->control.fields.unk2E5 >= 0x12)
    {
        func_800BD520(0, 0xA00, 1);
    }

    func_800BD520(0, 0x429C, D_80122B74->control.fields.unk2E6 & 0x7F);
    func_800BD520(0, 0x4300, D_800F0B48[D_80122B74->track_data[g_music_track_index][0]]);
    func_800BD520(0, 0x4304, D_800F0B48[D_80122B74->track_data[g_music_track_index][1]]);
    func_800BD520(0, 0x4308, D_800F0B48[D_80122B74->track_data[g_music_track_index][2]]);
    func_800BD520(0, 0x430C, D_800F0B48[D_80122B74->track_data[g_music_track_index][3]]);
    func_800BD520(0, 0x4310, D_800F0B48[D_80122B74->track_data[g_music_track_index][4]]);
    func_800BD520(0, 0x4314, D_800F0B48[D_80122B74->track_data[g_music_track_index][5]]);
    func_800BD520(0, 0x4318, D_800F0B48[D_80122B74->track_data[g_music_track_index][6]]);
    func_800BD520(0, 0x431C, D_800F0B48[D_80122B74->track_data[g_music_track_index][7]]);
    func_800BD520(0, 0x5320, func_800C3688(g_music_track_index));
    func_800BD520(0, 0x5328, g_music_track_index);
}
