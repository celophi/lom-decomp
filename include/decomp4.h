#ifndef _DECOMP4_H
#define _DECOMP4_H

#include "common.h"

extern s16 D_8003EC6A;
extern s32 D_8003EC60;
extern s32 D_8003EC70;
extern short D_8003EC64;
extern s32 D_8003EC68;
extern s32 D_8004F754;
extern s16 D_8003D47C;
extern s32 D_8004F7A0;
extern s16 D_8003EC42;
extern s32 D_8003EC3C;
extern s32 D_8003EC78;
extern s16 D_8003EC40;
extern s32 D_8003EC38;
extern s32 D_8003EC74;
extern u8 D_80049130[];
extern s32 D_8003EC24;
extern u32 D_8004F758;


extern u8 g_akao_xa_tracker[];
extern u8 g_sfx_channels[];

typedef struct
{
    u8 _pad0[8];
    s32 unk8;
} AkaoDriverFlags;

/* Struct for g_sfx_channels elements (size = 0x118) */
typedef struct SfxChannel
{
    u8 pad0[0x28];
    u32 field_28; /* offset 0x28, tested with 0x02000000 */
    u8 pad1[0x58 - 0x2C];
    u32 field_58; /* offset 0x58, counter */
    u8 pad2[0x66 - 0x5C];
    u16 field_66; /* offset 0x66, counter */
    u16 field_68; /* offset 0x68, counter */
    u8 pad3[0x118 - 0x6A];
} SfxChannel;

typedef struct AkaoSeqChannel
{
    u8 pad0[0x04];
    u32 unk4; /* offset 0x04 */
    u8 pad1[0x14 - 0x08];
    u32 unk14; /* offset 0x14 */
    u32 unk18; /* offset 0x18 */
    u32 unk1C; /* offset 0x1C */
    u8 pad3[0x5E - 0x20];
    u16 unk5E; /* offset 0x5E */
} AkaoSeqChannel;

/* Struct for D_8004D400 */
typedef struct {
    u32 unk0;           /* offset 0x00 */
    u8  pad0[0x08 - 0x04];
    u32 unk8;           /* offset 0x08 */
    u32 unkC;           /* offset 0x0C */
    u8  pad1[0x16 - 0x10];
    u16 unk16;          /* offset 0x16 */
    u32 unk18;          /* offset 0x18 */
} D_8004D400_t;

/* Struct for D_8003D160 */
typedef struct {
    u32 unk0;           /* offset 0x00 */
    u32 unk4;           /* offset 0x04 */
    u32 unk8;           /* offset 0x08 */
    u32 unkC;           /* offset 0x0C */
} D_8003D160_t;

extern s32 D_8003EC44;
extern u8 D_8003EC7A;
extern s32 D_8004D408;
extern s32 D_8003EC7C;
extern u16 D_8003EC1C;
extern s32 D_8004D40C;
extern s32             D_8003EC18;
extern AkaoSeqChannel  D_8004C260; 
extern D_8004D400_t    D_8004D400;
extern D_8003D160_t    D_8003D160;

extern AkaoSeqChannel* D_8003EC28;
extern AkaoChannelState* g_akao_seq_channel0;
extern AkaoDriverFlags g_akao_driver_flags;

#endif