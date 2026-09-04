#include "common.h"
typedef struct {
 s32 unk0; s32 unk4; s32 unk8; u32 unkC; s16 unk10,unk12,unk14,unk16; u8 unk18,unk19,unk1A; u8 pad1B[1]; s32 unk1C; u8 pad20[1]; u8 unk21,unk22,unk23,unk24,unk25; u8 pad26[1]; u8 unk27,unk28; u8 pad29[1]; s16 unk2A,unk2C; u16 unk2E; s16 unk30; u8 unk32,unk33,unk34,unk35,unk36,unk37,unk38; u8 pad39[1]; u8 unk3A,unk3B; u32 unk3C; s32 unk40; u32 unk44,unk48,unk4C; u8 pad50[4];
} FieldActorState;
typedef struct { u8 pad0[0x174]; u32 unk174; u8 pad178[0x23C-0x178]; } FieldActorSlot;
extern FieldActorSlot D_80105AE0[];
s32 func_80083EEC(u8, s32, s32);
void field_start_actor_animation(s32,s32,s32);
void func_8006C3FC(FieldActorState*);
void func_80086C00(u8);
void func_800869FC(FieldActorState *rec, s32 flag)
{
    u8 temp;
    if (flag != 0) {
        func_80083EEC(rec->unk3A, rec->unk3A + 0x40, 9);
        field_start_actor_animation(rec->unk3A + 0x40, 0, 0);
        temp = rec->unk21;
        if ((temp & 0x7F) != 0x1B) {
            rec->unk21 = (temp & 0x80) + 0x1B;
            rec->unk2E = 1;
            rec->unk27 = 0;
            rec->unk24 = 1;
            D_80105AE0[rec->unk3A].unk174 &= ~0x1800;
            func_8006C3FC(rec);
            rec->unk1C |= 0x800;
        }
        func_80086C00(rec->unk3A);
    }
}
