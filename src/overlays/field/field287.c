#include "common.h"

typedef struct
{
    u8  pad00[0x18];
    u32 unk18;   /* 0x18 */
    s16 unk1C;   /* 0x1C */
    s8  unk1E;   /* 0x1E */
    u8  pad1F;
    u32 unk20;   /* 0x20 */
    s16 unk24;   /* 0x24 */
    s8  unk26;   /* 0x26 */
    s8  unk27;   /* 0x27 */
    u8  pad28[0xCF - 0x28];
    s8  unkCF;   /* 0xCF */
} PadCtx;

extern u16 g_music_track_index;
extern PadCtx *g_pad_ctx;
extern u8 g_save_slot_index;

void func_800A8D10(s16 arg0, s8 arg1, s8 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    g_pad_ctx->unk24 = arg0;
    g_pad_ctx->unk26 = arg1;
    g_pad_ctx->unk27 = arg2;
    g_pad_ctx->unk18 = (g_pad_ctx->unk18 & 0xFE000000) | (arg3 & 0x01FFFFFF);
    g_pad_ctx->unk1C = (s16)arg4;
    g_pad_ctx->unk1E = (s8)arg5;
    g_pad_ctx->unk20 = (g_pad_ctx->unk20 & 0xFFFC0000) | g_music_track_index;
    g_pad_ctx->unkCF = (u8)g_save_slot_index;
}
