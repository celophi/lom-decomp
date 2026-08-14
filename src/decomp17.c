#include "common.h"
#include "psyq/libspu.h"

typedef struct
{
    u8  pad0[0x1C];     /* 0x00 - 0x1B */
    u32 reverb_mask;    /* 0x1C */
    u32 noise_mask;     /* 0x20 */
    u32 pitch_mod_mask; /* 0x24 */
} SfxControl;

typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u32 unk8; /* 0x08 */
} AkaoDriverFlags;

/* Configuration record passed by the caller (a0). */
typedef struct
{
    u8  pad0[0x10];  /* 0x00 - 0x0F */
    s32 unk10;       /* 0x10 */
    u8  pad14[0x04]; /* 0x14 - 0x17 */
    s32 unk18;       /* 0x18 */
    u16 unk1C;       /* 0x1C */
    u8  pad1E[0x02]; /* 0x1E - 0x1F */
    s32 unk20;       /* 0x20 */
} Cfg;

/* g_akao_xa_program_staging: streaming program parameters. */
typedef struct
{
    u8  pad0[0x10];  /* 0x00 - 0x0F */
    s32 unk10;       /* 0x10 */
    s32 unk14;       /* 0x14 */
    s32 unk18;       /* 0x18 flags */
    u16 unk1C;       /* 0x1C */
    u8  pad1E[0x02]; /* 0x1E - 0x1F */
    s32 unk20;       /* 0x20 */
    s32 unk24;       /* 0x24 */
} XaProgramStaging;

typedef struct
{
    s32 unk0;        /* 0x00 */
    u8  pad04[0x04]; /* 0x04 - 0x07 */
    s32 unk8;        /* 0x08 */
    s32 unkC;        /* 0x0C */
    s32 unk10;       /* 0x10 */
    u32 unk14;       /* 0x14 */
    s32 unk18;       /* 0x18 */
    u8  pad1C[0x10]; /* 0x1C - 0x2B */
    s32 unk2C;       /* 0x2C */
    u8  pad30[0x1C]; /* 0x30 - 0x4B */
    s32 unk4C;       /* 0x4C */
    u8  pad50[0x08]; /* 0x50 - 0x57 */
    s32 unk58;       /* 0x58 */
} XaTrk;

extern XaTrk g_akao_xa_tracker;
extern volatile s32 g_akao_spu_xfer_pending;
extern SfxControl g_akao_sfx_control;
extern AkaoDriverFlags g_akao_driver_flags;
extern XaProgramStaging g_akao_xa_program_staging;
extern u32 D_8004F76C[];

s32 func_8002D1C4(void);
void spu_set_key_off(u32 mask);
void spu_set_key_on(s32 mask);
void func_8002D4D8(s32 a0, s32 a1, s32 a2, s32 a3);
void func_8002DD08(void);
void func_8002DCDC(void);

/**
 * @see decomp.me (100%)
 */
void func_8002DDDC(Cfg *cfg, s32 a1, s32 a2, s32 a3)
{
    s32 ch;
    XaTrk *p;
    s32 mask;
    u32 val;
    u32 koff;

    ch = func_8002D1C4();
    if (ch == -1)
    {
        return;
    }

    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    p = &g_akao_xa_tracker;
    val = p->unkC;
    p->unk4C = a1;
    p->unk0 = (s32)((u8 *)cfg + 0x40);
    p->unk14 = cfg->unk10;
    mask = (1 << ch) | (1 << (ch + 1));
    koff = mask | val;
    p->unk18 = cfg->unk20;
    p->unk10 = ch;
    p->unkC = mask;
    spu_set_key_off(koff);
    p->unk8 = cfg->unk18;
    p->unk58 = cfg->unk1C;
    p->unk2C = a2;
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(a2);
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback((SpuTransferCallbackProc)func_8002DD08);

    val = p->unk14;
    if (val > 0x2000)
    {
        val = 0x2000;
    }
    SpuWrite((u_char *)p->unk0, val);
    p->unk0 += val;

    if (a3 != 0)
    {
        g_akao_sfx_control.noise_mask |= p->unkC;
    }
    else
    {
        g_akao_sfx_control.noise_mask &= ~p->unkC;
    }
    g_akao_sfx_control.pitch_mod_mask &= ~D_8004F76C[0];
    g_akao_sfx_control.reverb_mask &= ~D_8004F76C[0];
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @see decomp.me (100%)
 */
void func_8002DFA4(s32 arg0, s32 arg1)
{
    s32 ch;
    s32 val;
    s32 mask;
    XaProgramStaging *stg;

    if (g_akao_xa_program_staging.unk20 != 0)
    {
        ch = func_8002D1C4();
        if (ch != -1)
        {
            SpuSetIRQ(0);
            SpuSetIRQCallback(0);
            val = g_akao_xa_tracker.unkC;
            mask = (1 << ch) | (1 << (ch + 1));
            g_akao_xa_tracker.unk10 = ch;
            g_akao_xa_tracker.unkC = mask;
            spu_set_key_off(mask | val);
            stg = &g_akao_xa_program_staging;
            g_akao_xa_tracker.unk8 = stg->unk18;
            g_akao_xa_tracker.unk58 = stg->unk1C;
            if (arg1 != 0)
            {
                g_akao_sfx_control.noise_mask |= g_akao_xa_tracker.unkC;
            }
            else
            {
                g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
            }
            g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.unkC;
            g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.unkC;
            g_akao_driver_flags.unk8 |= 0x100;
            SpuSetIRQAddr(stg->unk20 + ((u32)stg->unk10 >> 1) + 8);
            SpuSetIRQCallback((SpuIRQCallbackProc)func_8002DCDC);

            if (stg->unk18 & 1)
            {
                if (stg->unk18 & 2)
                {
                    func_8002D4D8(g_akao_xa_tracker.unk10, 1, stg->unk20, stg->unk20 + stg->unk14);
                    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, stg->unk20 + stg->unk24, (stg->unk20 + stg->unk24) + stg->unk14);
                }
                else
                {
                    func_8002D4D8(g_akao_xa_tracker.unk10, 1, stg->unk20, 0x1030);
                    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, stg->unk20 + stg->unk24, 0x1030);
                }
            }
            else if (stg->unk18 & 2)
            {
                func_8002D4D8(g_akao_xa_tracker.unk10, 3, stg->unk20, stg->unk20 + stg->unk14);
                func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 3, stg->unk20, stg->unk20 + stg->unk14);
            }
            else
            {
                func_8002D4D8(g_akao_xa_tracker.unk10, 3, stg->unk20, 0x1030);
                func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 3, stg->unk20, 0x1030);
            }
            spu_set_key_on(D_8004F76C[0]);
            SpuSetIRQ(1);
        }
    }
}
