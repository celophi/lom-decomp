#include "common.h"

/** @brief D_80122B74 record; per-slot data at 0x8C stride, sub-slots at 0x10. */
typedef struct
{
    u8 pad[0x26F0];
    s32 unk26F0;
    u8 unk26F4;
} Rec;

extern u8 *D_80122B74;

extern s32 func_800C1E40(s32 arg0);
extern s32 func_800C0560(s32 arg0, s32 arg1, s32 arg2);
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @brief Populate the eight sub-slot handles for a field record, or fail audio.
 *
 * If func_800C1E40 reports no free channel, notifies the audio driver and
 * returns. Otherwise walks the eight 0x10-byte sub-slots of the @p arg0 record
 * (0x8C stride); each one still flagged 0xFF is resolved via func_800C0560 and
 * its handle stored into @c unk26F0.
 *
 * @param arg0 Field record index.
 * @see decomp.me (100%) TODO
 */
void func_800C0490(s32 arg0)
{
    s32 temp_v0;
    s32 sentinel;
    s32 var_s1;
    Rec *rec;

    temp_v0 = func_800C1E40(0x10);
    if (temp_v0 == 0)
    {
        akao_set_song_params(0x8001, 0x3E7, 0, 0);
        return;
    }
    var_s1 = 0;
    do
    {
        rec = (Rec *)(D_80122B74 + (arg0 * 0x8C + var_s1 * 0x10));
        if (rec->unk26F4 != (sentinel = 0xFF))
        {
            sentinel = temp_v0;
            ((Rec *)(D_80122B74 + (arg0 * 0x8C + var_s1 * 0x10)))->unk26F0 = func_800C0560(arg0, var_s1, sentinel);
        }
        var_s1 += 1;
    } while (var_s1 < 8);
}
