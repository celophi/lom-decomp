#ifndef _AKAO_DRIVER_H
#define _AKAO_DRIVER_H

#include "common.h"
#include "akao.h"

extern s32 g_akao_spu_xfer_pending;
extern u8 g_akao_articulation_slots[];
extern u8 g_sfx_channels[];
extern s32 D_8003EC7C;
extern s32 D_8003EC6C;
extern s32 D_8003EC44;
extern u8 g_akao_xa_tracker[];
extern s16 D_8003EC64;
extern s32 D_8003EC78;
extern s16 D_8003EC42;
extern s32 D_8003EC74;
extern s16 D_8003EC40;
extern s32 D_8003EC70;
extern s32 D_8003EC68;
extern s32 D_8003EC24;
extern AkaoChannelState* g_akao_seq_channel1;
extern AkaoChannelState *g_akao_seq_channel0;
extern void *D_8003EC58;
extern u8 D_8004C2D0[];
extern u8 g_akao_sfx_control[];
extern u8 D_8004F830[];
extern u8 g_akao_driver_flags[];
extern u8 D_8004D388[];
extern u8 D_8003EC30[];
extern u8 g_akao_seq_channels[];
extern u8 g_akao_seq_master_state[];
extern char g_akao_spu_malloc_table[];
extern char g_akao_spu_zero_primer[];
extern s32 g_akao_rcnt2_event;
extern s32 D_8003EC48;
extern s32 D_8003EC50;
extern s32 D_8003EC54;

extern s32 akao_check_magic(s32 *data);
extern void func_80028E34(int, volatile short, void *, int);
extern void func_8002A134(void);

inline static u8* off(u8* p, int o)
{
    return p + o;
}

#endif