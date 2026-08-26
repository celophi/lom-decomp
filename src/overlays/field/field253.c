#include "common.h"

typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
    u32 unkC; /* 0x0C */
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
    s16 unk16; /* 0x16 */
    u8 unk18; /* 0x18 */
    u8 unk19; /* 0x19 */
    u8 unk1A; /* 0x1A */
    u8 pad1B[0x1C - 0x1B];
    s32 unk1C; /* 0x1C */
    u8 pad20[0x21 - 0x20];
    u8 unk21; /* 0x21 */
    u8 unk22; /* 0x22 */
    u8 unk23; /* 0x23 */
    u8 unk24; /* 0x24 */
    u8 unk25; /* 0x25 */
    u8 pad26[0x27 - 0x26];
    u8 unk27; /* 0x27 */
    u8 unk28; /* 0x28 */
    u8 pad29[0x2A - 0x29];
    s16 unk2A; /* 0x2A */
    s16 unk2C; /* 0x2C */
    u16 unk2E; /* 0x2E */
    s16 unk30; /* 0x30 */
    u8 unk32; /* 0x32 */
    u8 unk33; /* 0x33 */
    u8 unk34; /* 0x34 */
    u8 unk35; /* 0x35 */
    u8 unk36; /* 0x36 */
    u8 unk37; /* 0x37 */
    u8 unk38; /* 0x38 */
    u8 pad39[0x3A - 0x39];
    u8 unk3A; /* 0x3A */
    u8 unk3B; /* 0x3B */
    u32 unk3C; /* 0x3C */
    s32 unk40; /* 0x40 */
    u32 unk44; /* 0x44 */
    u32 unk48; /* 0x48 */
    u32 unk4C; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u8 unk8; /* 0x08 */
    u8 unk9; /* 0x09 */
    u8 padA;
    u8 unkB; /* 0x0B */
    u8 unkC; /* 0x0C */
    u8 unkD; /* 0x0D */
    u8 unkE; /* 0x0E */
    u8 unkF; /* 0x0F */
    u8 unk10; /* 0x10 */
    u8 unk11; /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 unk14; /* 0x14 (overlaps unk16 at its upper halfword) */
    s16 unk18; /* 0x18 */
    u8 pad1A[0x23 - 0x1A];
    u8 unk23; /* 0x23 */
    u32 unk24; /* 0x24 (overlaps byte writes at 0x24/0x25) */
    u32 unk28; /* 0x28 */
    u8 unk2C; /* 0x2C */
    u8 pad2D;
    u8 unk2E; /* 0x2E */
    u8 pad2F[0x31 - 0x2F];
    u8 unk31; /* 0x31 */
    u8 pad32;
    u8 unk33; /* 0x33 */
    u32 unk34; /* 0x34 */
    u8 pad38[0x48 - 0x38];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

void func_8008699C(Struct_D800FDF58 *rec, s32 flag)
{
    if (flag != 0)
    {
        D_800FE3A0[rec->unk3A].unk2E = 0x20;
        D_800FE3A0[rec->unk3A].unk33 = 0x20;
    }
    else
    {
        D_800FE3A0[rec->unk3A].unk2E = 0x40;
        D_800FE3A0[rec->unk3A].unk33 = 0x40;
    }
}
