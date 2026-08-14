#include "common.h"
#include "psyq/libspu.h"

/* Streamed SPU buffer; AKAO magic sits at +0x80. */
typedef struct
{
    u8  pad0[0x80]; /* 0x00 - 0x7F */
    u32 unk80;      /* 0x80 - magic 'AKAO' */
} AkaoBuf;

/* Sub-header located at +0x80 inside the streamed SPU buffer. */
typedef struct
{
    u32 unk0;       /* 0x00 (buffer +0x80) magic */
    u32 unk4;       /* 0x04 (buffer +0x84) */
    u8  pad0[0x08]; /* 0x08 - 0x0F */
    u32 unk10;      /* 0x10 (buffer +0x90) */
    u8  pad1[0x0C]; /* 0x14 - 0x1F */
    u32 unk20;      /* 0x20 (buffer +0xA0) */
} AkaoStreamHdr;

/* Source descriptor consumed by func_8002E75C. */
typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
} AkaoStreamSrc;

/*
 * Reconciled superset of the AkaoXaTracker layouts used by the functions in
 * this file (func_8002E540 unk10; func_8002E5A4 unk0/unkC/unk10/unk18/unk20/
 * unk2C/unk34; func_8002E75C unk8/unk2C/unk30). All accessed offsets and
 * types agree; only the accessed fields are load-bearing for codegen.
 */
typedef struct
{
    AkaoBuf *unk0;  /* 0x00 - current buffer pointer */
    u8  pad0[0x04]; /* 0x04 - 0x07 */
    u32 unk8;       /* 0x08 */
    s32 unkC;       /* 0x0C - XA stream active flag */
    s32 unk10;      /* 0x10 - base SPU voice index */
    u8  pad1[0x04]; /* 0x14 - 0x17 */
    u32 unk18;      /* 0x18 */
    u8  pad2[0x04]; /* 0x1C - 0x1F */
    u32 unk20;      /* 0x20 */
    u8  pad3[0x08]; /* 0x24 - 0x2B */
    AkaoBuf *unk2C; /* 0x2C */
    u32 unk30;      /* 0x30 */
    s32 unk34;      /* 0x34 */
} AkaoXaTracker;

extern AkaoXaTracker g_akao_xa_tracker;

void func_8002D4D8(int a0, int a1, int a2, int a3);
void func_8002E478(int a0, int a1, void (*cb)());
s32 func_8002D140(AkaoStreamSrc *arg0);
void akao_spu_arm_xfer(void);
void func_8002E294(void *arg0);
void spu_set_voice_repeat_addr(s32 voice, s32 addr);
void func_8002E6FC(void);
void func_8002E72C(void);

/**
 * @see decomp.me (100%)
 */
void func_8002E540(void)
{
    func_8002D4D8(g_akao_xa_tracker.unk10, 1, 0x1100, 0x2100);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, 0x1900, 0x2900);
    func_8002E478(0x2000, 0x2100, func_8002E6FC);
}

/**
 * @see decomp.me (100%)
 */
void func_8002E5A4(s32 arg0, s32 arg1, s32 arg2, void (*arg3)())
{
    AkaoBuf *base;
    AkaoStreamHdr *hdr;

    if (g_akao_xa_tracker.unkC != 0)
    {
        base = g_akao_xa_tracker.unk0;
        hdr = (AkaoStreamHdr *)((u8 *)base + 0x80);
        if (base->unk80 == 0x4F414B41)
        {
            SpuSetIRQ(0);
            SpuSetTransferStartAddr(arg0);
            akao_spu_arm_xfer();
            func_8002E294(&g_akao_xa_tracker.unk34);
            SpuWrite((u8 *)g_akao_xa_tracker.unk0 + 0xD0, arg2 - 0xD0);
            g_akao_xa_tracker.unk20 = hdr->unk4;
            g_akao_xa_tracker.unk18 = hdr->unk20;
            if (hdr->unk10 > hdr->unk20)
            {
                SpuSetIRQCallback(arg3);
                g_akao_xa_tracker.unk0 = (AkaoBuf *)((u8 *)g_akao_xa_tracker.unk0 + arg2);
                if (g_akao_xa_tracker.unk34 == 0)
                {
                    g_akao_xa_tracker.unk0 = g_akao_xa_tracker.unk2C;
                }
            }
            else
            {
                SpuSetIRQCallback(func_8002D140);
                arg1 = 0x1030;
                arg0 = 0x1030;
            }
            spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10, arg0);
            spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10 + 1, arg1);
            SpuSetIRQAddr(arg0 + 8);
            SpuSetIRQ(1);
        }
    }
}

/**
 * @see decomp.me (100%)
 */
void func_8002E6FC(void)
{
    func_8002E5A4(0x1100, 0x1900, 0x1000, func_8002E72C);
}

/**
 * @see decomp.me (100%)
 */
void func_8002E72C(void)
{
    func_8002E5A4(0x2100, 0x2900, 0x1000, func_8002E6FC);
}

/**
 * @see decomp.me (100%)
 */
void func_8002E75C(AkaoStreamSrc *arg0)
{
    AkaoStreamSrc *src = arg0;

    func_8002D140(arg0);
    g_akao_xa_tracker.unk8 = 0x1000000;
    g_akao_xa_tracker.unk2C = src->unk0;
    g_akao_xa_tracker.unk30 = src->unk4;
}
