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
extern void* D_8003EC28;
extern s32 D_8003EC24;
extern s32 D_8004D400;

extern u8 g_akao_xa_tracker[];
extern u8 g_sfx_channels[];

typedef struct
{
    u8 _pad0[8];
    s32 unk8;
} AkaoDriverFlags;

typedef struct AkaoChannelState
{
    u32 flags;       /* 0x00 */
    u32 unk4;        /* 0x04 */
    u8 _pad08[0x10]; /* 0x08 – 0x17 */
    u32 unk18;       /* 0x18 */
    u8 _pad1C[0x04]; /* 0x1C – 0x1F */
    u32 unk20;       /* 0x20 (low 16 bits + high 16 bits = unk22) */
    u32 unk24;       /* 0x24 */
    u32 unk28;       /* 0x28 */
    u8 _pad2C[0x1C]; /* 0x2C – 0x47 */
    u32 unk48;       /* 0x48 */
    u32 unk4C;       /* 0x4C */
    u8 _pad50[0x0A]; /* 0x50 – 0x59 */
    s16 unk5A;       /* 0x5A */
    u16 unk5C;       /* 0x5C */
    u8 _pad5E[0x06]; /* 0x5E – 0x63 */
    u16 unk64;       /* 0x64 */
    u16 unk66;       /* 0x66 */
    u16 unk68;       /* 0x68 */
    u16 unk6A;       /* 0x6A */
    u16 unk6C;       /* 0x6C */
    u8 _pad6E[0xAA]; /* 0x6E – 0x117 */
} AkaoChannelState;  /* total = 0x118 */

extern s32 D_8003EC44;
extern u8 D_8003EC7A;
extern s32 D_8003EC7C;
extern AkaoChannelState* g_akao_seq_channel0;
extern AkaoDriverFlags g_akao_driver_flags;

#endif