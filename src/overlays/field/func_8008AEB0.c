#include "common.h"

typedef struct { u8 pad0[0x14]; s32 unk14; u8 pad18[0x224]; } ActorAEB0;
typedef struct { u8 pad0[0x1C]; u32 unk1C; u8 pad20[0xA]; s16 unk2A; u8 pad2C[0x28]; } RecAEB0;
extern ActorAEB0 D_80105AE0[];
extern RecAEB0 D_800FDF58[];

s32 func_8008AEB0(s32 id)
{
    s32 v;
    s32 result;
    s32 masked;
    RecAEB0 *rec;
    ActorAEB0 *actor;

    rec = D_800FDF58;
    actor = D_80105AE0;
    v = 0;
    while (v < 13) {
        if (actor->unk14 == id) {
            v = (s32)rec;
            goto found_done;
        }
        v++;
        actor++;
        rec++;
    }
    v = -1;
found_done:
    if (v == -1) {
        return -1;
    }
    if (((*(u16 *)&((RecAEB0 *)v)->unk1C) & 0x1FF) < 2) {
        result = 0;
        if (((RecAEB0 *)v)->unk2A == 0) {
            masked = ((RecAEB0 *)v)->unk1C & 0x600;
            result = masked == 0;
        }
    } else {
        result = 0;
        if ((((RecAEB0 *)v)->unk2A == 0x81) || (((RecAEB0 *)v)->unk2A == 0)) {
            result = 1;
        }
    }
    return result;
}
