#include "common.h"
#include "psyq/libspu.h"

typedef struct
{
    u32 unk0;      /* 0x00 */
    s32 unk4;      /* 0x04 */
    u32 unk8;      /* 0x08 */
    u32 unkC;      /* 0x0C */
    u32 unk10;     /* 0x10 */
    u8  _pad14[2]; /* 0x14 - 0x15 */
    u16 unk16;     /* 0x16 */
    u32 unk18;     /* 0x18 */
    u32 reverb_mask;    /* 0x1C */
    u32 noise_mask;     /* 0x20 */
    u32 pitch_mod_mask; /* 0x24 */
    u16 unk28;
} SfxControl;

typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u32 unk8; /* 0x08 */
} AkaoDriverFlags;

/* SPU source header pointed to by XaTrk.unk2C (fields accessed at +0x80). */
typedef struct
{
    u8  pad0[0x10]; /* 0x00 - 0x0F */
    s32 unk10;      /* 0x10 */
    u8  pad14[0x04];/* 0x14 - 0x17 */
    s32 unk18;      /* 0x18 */
    u16 unk1C;      /* 0x1C */
    u8  pad1E[0x02];/* 0x1E - 0x1F */
    s32 unk20;      /* 0x20 */
    u8  pad24[0x04];/* 0x24 - 0x27 */
    s32 unk28;      /* 0x28 */
} XaSrc;

/*
 * XA-streaming tracker. Superset view reconciled from func_8002E2E8's extended
 * layout (reaches 0x58, unk2C is a pointer) and func_8002E478's fields.
 * unk14 is kept unsigned so func_8002E478's `unk14 >= 0xE61` stays an unsigned
 * compare; func_8002E2E8 only stores to unk14, so the type change is neutral there.
 */
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
    u8 *unk2C;       /* 0x2C */
    u8  pad30[0x04]; /* 0x30 - 0x33 */
    s32 unk34;       /* 0x34 */
    u8  pad38[0x08]; /* 0x38 - 0x3F */
    s32 unk40;       /* 0x40 */
    u8  pad44[0x04]; /* 0x44 - 0x47 */
    s32 unk48;       /* 0x48 */
    u8  pad4C[0x0C]; /* 0x4C - 0x57 */
    s32 unk58;       /* 0x58 */
} XaTrk;

extern XaTrk g_akao_xa_tracker;
extern s32 g_akao_spu_xfer_pending;
extern SfxControl g_akao_sfx_control;
extern AkaoDriverFlags g_akao_driver_flags;
extern s32 D_8004F76C[];

s32 func_8002D1C4(void);
void akao_cmd_e5(s32 a0, s32 a1);
void spu_set_key_off(u32 mask);
void func_8002E294(void *p);
void func_8002E540(void);
void func_8002D140(void);
void spu_set_key_on(u32 voice_mask);

/**
 * @see decomp.me (97.89%)
 */
void func_8002E2E8(void)
{
    s32 ch;
    XaTrk *p;
    XaSrc *src;
    s32 mask;
    s32 m2;

    ch = func_8002D1C4();
    if (ch == -1)
    {
        return;
    }

    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    src = (XaSrc *)(g_akao_xa_tracker.unk2C + 0x80);

    if (src->unk28 != 0 && g_akao_xa_tracker.unk48 == 0)
    {
        s32 pan = g_akao_xa_tracker.unk40;
        g_akao_xa_tracker.unk40 = 0;
        akao_cmd_e5(src->unk28, pan >> 8);
    }

    p = &g_akao_xa_tracker;
    g_akao_xa_tracker.unk0 = (s32)p->unk2C;
    p->unk14 = src->unk10;
    p->unk18 = src->unk20;
    p->unk10 = ch;
    mask = (1 << ch) | (1 << (ch + 1));
    p->unkC = mask;
    p->unk34 = 0;
    spu_set_key_off(mask);
    p->unk8 = src->unk18;
    p->unk58 = src->unk1C;
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(0x1100);
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback((SpuTransferCallbackProc)func_8002E540);
    SpuWrite((u_char *)(g_akao_xa_tracker.unk0 + 0xD0), 0x2000);

    m2 = p->unkC;
    g_akao_sfx_control.noise_mask &= ~m2;
    g_akao_sfx_control.pitch_mod_mask &= ~m2;
    g_akao_sfx_control.reverb_mask &= ~m2;
    g_akao_driver_flags.unk8 |= 0x100;

    func_8002E294(&p->unk34);
    func_8002E294(&p->unk34);
}

/**
 * @see decomp.me (100%)
 */
void func_8002E478(s32 a0, s32 a1, void (*cb)())
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetTransferCallback(0);
        g_akao_spu_xfer_pending = 0;
        if (g_akao_xa_tracker.unk14 >= 0xE61)
        {
            g_akao_xa_tracker.unk0 += a0;
            SpuSetIRQCallback(cb);
        }
        else
        {
            SpuSetIRQCallback(func_8002D140);
            a1 = 0x1030;
        }
        SpuSetIRQAddr(a1 + 8);
        spu_set_key_on(D_8004F76C[0]);
        SpuSetIRQ(1);
    }
}
