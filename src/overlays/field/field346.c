#include "common.h"

typedef struct {
    u8 pad0[4];
    s32 unk4;
} SubState;

typedef struct {
    u8 pad0[3];
    u8 unk3;
} Entry;

typedef struct {
    u8 pad0[0x18];
    SubState *unk18;
    Entry *unk1C;
} FieldState;

typedef s32 (*Handler)(void);

extern FieldState *D_80123FB0;
extern Handler D_800F0B98[];
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @brief Advance the active field command through its per-opcode handler.
 *
 * When the field state has a current entry, dispatches its opcode (@c unk3):
 * opcodes below 8 run their handler from the func_800F0B98 table (returning its
 * result); higher opcodes latch the song via akao_set_song_params. Returns 0
 * when there is no entry or after the song latch.
 *
 * @return The dispatched handler's result, or 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800B6808(void)
{
    Entry *e;

    e = D_80123FB0->unk1C;
    if (e != NULL)
    {
        if (e->unk3 < 8)
        {
            return D_800F0B98[e->unk3]();
        }
        akao_set_song_params(0x8001, 0x65, e->unk3, D_80123FB0->unk18->unk4);
        return 0;
    }
    return 0;
}
