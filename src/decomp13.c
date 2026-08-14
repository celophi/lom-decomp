#include "common.h"
#include "psyq/libspu.h"

typedef struct
{
    u8  *unk0;        /* 0x00 - buffer base */
    s32  unk4;        /* 0x04 */
    u32  unk8;        /* 0x08 - flags */
    s32  unkC;        /* 0x0C - voice mask */
    s32  unk10;       /* 0x10 - channel slot */
    s32  unk14;       /* 0x14 */
    s32  unk18;       /* 0x18 */
    s32  unk1C;       /* 0x1C */
    u8   pad20[0x2C]; /* 0x20 - 0x4B */
    s32  unk4C;       /* 0x4C */
    u8   pad50[0x08]; /* 0x50 - 0x57 */
    s32  unk58;       /* 0x58 */
} XaTracker;

typedef struct
{
    u8   pad0[0x10];
    s32  unk10;
    s32  unk14;
    s32  unk18;
    u16  unk1C;
    u8   pad1E[0x02];
    s32  unk20;
} SeqSrc;

typedef struct
{
    u8   pad0[0x1C];
    u32  reverb_mask;   /* 0x1C */
    u32  noise_mask;    /* 0x20 */
    u32  pitch_mod_mask;/* 0x24 */
} SfxControl;

typedef struct
{
    u32  unk0;
    u32  unk4;
    u32  unk8;
} DriverFlags;

extern XaTracker g_akao_xa_tracker;
extern SfxControl g_akao_sfx_control;
extern DriverFlags g_akao_driver_flags;
extern s32 g_akao_spu_xfer_pending;
extern u32 D_8004F76C[];

s32 func_8002D1C4(void);
void spu_set_key_off(u32 voice_mask);
void func_8002D7C8(void);
void func_8002D254(void);

/**
 * @see decomp.me (99.51%)
 */
void func_8002D29C(void *arg0, s32 arg1, s32 flag)
{
    SeqSrc *hdr;
    s32 slot;
    s32 mask;
    s32 xfer;
    s32 size;
    s32 lead;
    u32 nmask;
    XaTracker *t;
    SfxControl *sc;

    slot = func_8002D1C4();
    if (slot == -1)
    {
        return;
    }

    g_akao_xa_tracker.unk4C = arg1;
    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    hdr = arg0;
    g_akao_xa_tracker.unk0 = (u8 *)hdr + 0x40;
    g_akao_xa_tracker.unk14 = hdr->unk10;
    mask = (1 << slot) | (1 << (slot + 1));

    g_akao_xa_tracker.unk18 = hdr->unk20;
    g_akao_xa_tracker.unk10 = slot;
    g_akao_xa_tracker.unkC = mask;
    spu_set_key_off(mask);
    g_akao_xa_tracker.unk8 = hdr->unk18;

    g_akao_xa_tracker.unk58 = (u16)hdr->unk1C;
    g_akao_xa_tracker.unk10 = slot;
    g_akao_xa_tracker.unkC = mask;
    spu_set_key_off(mask);

    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(0x1100);
    g_akao_spu_xfer_pending = 1;

    if (g_akao_xa_tracker.unk8 & 2)
    {
        xfer = (s32)g_akao_xa_tracker.unk0 + hdr->unk14;
    }
    else
    {
        xfer = 0;
    }

    t = &g_akao_xa_tracker;
    t->unk4 = xfer;

    if (t->unk8 & 1)
    {
        if (t->unk8 & 2)
        {
            lead = t->unk14 - ((u32)hdr->unk14 >> 1);
        }
        else
        {
            lead = 0;
        }
        t->unk1C = lead;
        SpuSetTransferCallback(func_8002D7C8);
        size = 0x2000;
    }
    else
    {
        if (t->unk8 & 2)
        {
            lead = t->unk14 - hdr->unk14;
        }
        else
        {
            lead = 0;
        }
        t->unk1C = lead;
        SpuSetTransferCallback(func_8002D254);
        size = 0x800;
    }

    SpuWrite((void *)g_akao_xa_tracker.unk0, size);

    if (flag != 0)
    {
        g_akao_sfx_control.noise_mask |= g_akao_xa_tracker.unkC;
    }
    else
    {
        g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
    }

    sc = &g_akao_sfx_control;
    nmask = ~D_8004F76C[0];
    sc->pitch_mod_mask &= nmask;
    sc->reverb_mask &= nmask;
    g_akao_driver_flags.unk8 |= 0x100;
}
