#include "common.h"
#include "psyq/libspu.h"

/*
 * Several functions in this file view the single global g_akao_xa_tracker
 * through different, mutually incompatible struct layouts (SPU stream state,
 * pan-fade state, XA transfer state). A file-scope extern of any one view
 * would conflict with the others, so each such view (typedef + extern) is
 * declared block-scoped inside the function that was matched under it.
 */

/* ---- shared command / parameter structs (no layout conflicts) ---- */

/** @brief SFX channel control bitfields (size 0x28). */
typedef struct
{
    u32 unk0;      /* 0x00 -- active-channel bitmask */
    s32 unk4;      /* 0x04 */
    u32 unk8;      /* 0x08 */
    u32 unkC;      /* 0x0C */
    u32 unk10;     /* 0x10 */
    u8 _pad14[2];  /* 0x14 - 0x15 */
    u16 unk16;     /* 0x16 -- tick step */
    u32 unk18;     /* 0x18 -- tick accumulator */
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
} AkaoCmd0C;

typedef struct
{
    s32 unk0;
} AkaoCmd;

typedef struct
{
    u32  unk0;       /* 0x00 - fade duration in ticks (0 => 1) */
    u16  unk4;       /* 0x04 - target pan */
} AkaoXaCmd;

/* ---- shared externs (no conflicts) ---- */
extern SfxControl g_akao_sfx_control;
extern s32 D_8004F76C[];
extern u32 D_8004F754;
extern s16 D_8003D47C;
extern s16 D_8003D37C[];

/* ---- callee prototypes ---- */
void func_8002D140(void);
void func_8002D29C(u32 arg0, u32 arg1, u32 arg2);
void func_8002D4D8(int a0, int a1, int a2, int a3);
void func_8002D694(int a0, int a1, void (*cb)());
void akao_spu_arm_xfer(void);
void akao_spu_write(s32 src_addr, s32 byte_count);
void spu_set_voice_repeat_addr(s32 voice, u32 addr);
void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale);
void spu_set_key_on(u32 voice_mask);

/* ---- forward declarations for functions defined later in this file ---- */
void func_8002D978(void);
void func_8002D9A8(void);
void func_8002D9D8(void);
void func_8002DA08(void);
void func_8002DCDC(void);

/**
 * @see decomp.me (100%)
 */
void func_8002D764(void)
{
    typedef struct
    {
        u8   pad0[0x08];
        u32  unk8;
        s32  unkC;
        s32  unk10;
        u8   pad1[0x0C];
        s32  unk20;
        s32  unk24;
        s32  unk28;
        u8   pad2[0x08];
        s32  unk34;
        s32  unk38;
        s32  unk3C;
        s32  unk40;
        s32  unk44;
        s32  unk48;
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    func_8002D4D8(g_akao_xa_tracker.unk10, 0, 0x1100, 0x2100);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 0, 0x1100, 0x2100);
    func_8002D694(0x1000, 0x2100, func_8002D978);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D7C8(void)
{
    typedef struct
    {
        u8   pad0[0x08];
        u32  unk8;
        s32  unkC;
        s32  unk10;
        u8   pad1[0x0C];
        s32  unk20;
        s32  unk24;
        s32  unk28;
        u8   pad2[0x08];
        s32  unk34;
        s32  unk38;
        s32  unk3C;
        s32  unk40;
        s32  unk44;
        s32  unk48;
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    func_8002D4D8(g_akao_xa_tracker.unk10, 1, 0x1100, 0x2100);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, 0x1900, 0x2900);
    func_8002D694(0x2000, 0x2100, func_8002D9D8);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D82C(u32 start, u32 end, u32 size, void (*cb)())
{
    typedef struct
    {
        u8  *unk0;       /* 0x00 - SPU write address */
        u8  *unk4;       /* 0x04 - next chunk address */
        u8   pad0[0x04]; /* 0x08 - 0x0B */
        s32  unkC;       /* 0x0C - stream active flag */
        s32  unk10;      /* 0x10 - base SPU voice index */
        u32  unk14;      /* 0x14 - remaining byte counter */
        u8   pad1[0x04]; /* 0x18 - 0x1B */
        u32  unk1C;      /* 0x1C - reload counter value */
        u8   pad2[0x2C]; /* 0x20 - 0x4B */
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    if (g_akao_xa_tracker.unkC == 0)
    {
        return;
    }
    if (g_akao_xa_tracker.unk14 == 0)
    {
        return;
    }

    SpuSetTransferStartAddr(start);
    akao_spu_arm_xfer();
    SpuWrite(g_akao_xa_tracker.unk0, size);
    SpuSetIRQ(0);

    if (g_akao_xa_tracker.unk14 > 0x800)
    {
        SpuSetIRQCallback(cb);
        g_akao_xa_tracker.unk14 -= 0x800;
        g_akao_xa_tracker.unk0 += size;
    }
    else if (g_akao_xa_tracker.unk4 != 0)
    {
        SpuSetIRQCallback(cb);
        g_akao_xa_tracker.unk0 = g_akao_xa_tracker.unk4;
        g_akao_xa_tracker.unk14 = g_akao_xa_tracker.unk1C;
    }
    else
    {
        SpuSetIRQCallback(func_8002D140);
        end = 0x1030;
        start = end;
        g_akao_xa_tracker.unk14 = 0;
    }

    spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10, start);
    spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10 + 1, end);
    SpuSetIRQAddr(start + 8);
    SpuSetIRQ(1);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D978(void)
{
    func_8002D82C(0x1100, 0x1100, 0x800, func_8002D9A8);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D9A8(void)
{
    func_8002D82C(0x2100, 0x2100, 0x800, func_8002D978);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D9D8(void)
{
    func_8002D82C(0x1100, 0x1900, 0x1000, func_8002DA08);
}

/**
 * @see decomp.me (100%)
 */
void func_8002DA08(void)
{
    func_8002D82C(0x2100, 0x2900, 0x1000, func_8002D9D8);
}

/**
 * @see decomp.me (100%)
 */
void func_8002DA38(AkaoCmd0C *p)
{
    func_8002D29C(p->unk0, p->unk4, p->unk8);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @see decomp.me (100%)
 */
void func_8002DA80(void)
{
    func_8002D140();
}

/**
 * @see decomp.me (100%)
 */
void func_8002DAA0(AkaoCmd *cmd)
{
    typedef struct
    {
        u8   pad0[0x08];
        u32  unk8;
        s32  unkC;
        s32  unk10;
        u8   pad1[0x0C];
        s32  unk20;
        s32  unk24;
        s32  unk28;
        u8   pad2[0x08];
        s32  unk34;
        s32  unk38;
        s32  unk3C;
        s32  unk40;
        s32  unk44;
        s32  unk48;
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    AkaoXaTracker *xa = &g_akao_xa_tracker;
    s32 val = cmd->unk0;

    xa->unk48 = 0;
    xa->unk40 = val;
    if (xa->unkC != 0)
    {
        spu_set_voice_volume(xa->unk10, (val << 15) >> 16, 0, 0);
        spu_set_voice_volume(xa->unk10 + 1, 0, (xa->unk40 << 15) >> 16, 0);
    }
}

/**
 * @see decomp.me (100%)
 */
void func_8002DB10(AkaoXaCmd *p)
{
    typedef struct
    {
        u8   pad[0x40];  /* 0x00 - 0x3F */
        u16  unk40;      /* 0x40 - pan accumulator (low half) */
        u16  pad42;      /* 0x42 */
        s32  unk44;      /* 0x44 - pan step */
        s32  unk48;      /* 0x48 - pan fade-tick countdown */
    } XaTracker;
    extern XaTracker g_akao_xa_tracker;

    s16 den;
    s16 num;

    den = 1;
    if (p->unk0 != 0)
    {
        den = p->unk0;
    }
    num = p->unk4 - g_akao_xa_tracker.unk40;
    g_akao_xa_tracker.unk44 = (s16)(num / den);
    g_akao_xa_tracker.unk48 = den;
}

/**
 * @see decomp.me (100%)
 */
void func_8002DB90(AkaoCmd *cmd)
{
    typedef struct
    {
        u8   pad0[0x08];
        u32  unk8;
        s32  unkC;
        s32  unk10;
        u8   pad1[0x2C];
        s32  unk40;
        s32  unk44;
        s32  unk48;
        s32  unk4C;
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    AkaoXaTracker *xa;
    s32 val;
    s32 idx;
    s32 temp_s0;
    s32 temp_s1;
    s32 temp_a;

    xa = &g_akao_xa_tracker;
    val = cmd->unk0;
    xa->unk4C = val;
    if (xa->unkC != 0)
    {
        if (D_8004F754 & 2)
        {
            temp_a = (xa->unk40 * D_8003D47C) >> 16;
            spu_set_voice_volume(xa->unk10, temp_a, temp_a, 0);
            spu_set_voice_volume(xa->unk10 + 1, temp_a, temp_a, 0);
        }
        else if (xa->unk8 & 1)
        {
            temp_a = xa->unk40;
            temp_a <<= 15;
            temp_a >>= 16;
            spu_set_voice_volume(xa->unk10, temp_a, 0, 0);
            spu_set_voice_volume(xa->unk10 + 1, 0, temp_a, 0);
        }
        else
        {
            idx = (val >> 8) & 0xFF;
            temp_s0 = (xa->unk40 * D_8003D37C[idx]) >> 16;
            idx ^= 0xFF;
            temp_s1 = (xa->unk40 * D_8003D37C[idx]) >> 16;
            spu_set_voice_volume(xa->unk10, temp_s0, temp_s1, 0);
            spu_set_voice_volume(xa->unk10 + 1, temp_s0, temp_s1, 0);
        }
    }
}

/**
 * @see decomp.me (100%)
 */
void func_8002DCDC(void)
{
    SpuSetIRQAddr(0x1038);
    SpuSetIRQCallback(func_8002D140);
}

/**
 * @see decomp.me (100%)
 */
void func_8002DD08(void)
{
    typedef struct
    {
        s32  buf;        /* 0x00 - SPU source addr */
        u8   pad0[0x08]; /* 0x04 - 0x0B */
        s32  unkC;       /* 0x0C - key-on voice mask */
        s32  unk10;      /* 0x10 - base SPU voice index */
        u32  unk14;      /* 0x14 - byte count */
        u8   pad1[0x14]; /* 0x18 - 0x2B */
        s32  unk2C;      /* 0x2C - SPU base addr */
    } XaTracker;
    extern XaTracker g_akao_xa_tracker;

    s32 addr;

    if (g_akao_xa_tracker.unk14 > 0x2000)
    {
        SpuSetTransferStartAddr(g_akao_xa_tracker.unk2C + 0x2000);
        akao_spu_write(g_akao_xa_tracker.buf, g_akao_xa_tracker.unk14 - 0x2000);
        addr = g_akao_xa_tracker.unk2C + 0x1FF8;
    }
    else
    {
        addr = g_akao_xa_tracker.unk2C + (g_akao_xa_tracker.unk14 >> 1) + 8;
    }

    func_8002D4D8(g_akao_xa_tracker.unk10, 0, g_akao_xa_tracker.unk2C, 0x1030);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 0, g_akao_xa_tracker.unk2C, 0x1030);
    SpuSetIRQAddr(addr);
    SpuSetIRQCallback(func_8002DCDC);
    spu_set_key_on(g_akao_xa_tracker.unkC);
    SpuSetIRQ(1);
}
