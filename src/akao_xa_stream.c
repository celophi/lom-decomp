/*
 * akao_xa_stream.c - consolidated AKAO XA/SPU streaming region (asm/unk8.s).
 *
 * All 36 functions from akao_seq_op_obey_voice_reserve (0x8002D0DC) through func_8002E75C live here, in
 * address order, built as a single gcc280_g4 object. The region drives one
 * global, g_akao_xa_tracker, viewed through many mutually incompatible struct
 * layouts (SPU stream state, pan-fade state, XA transfer state, ...). A single
 * file-scope extern of any one view would poison the others, so each such view
 * (typedef + `extern ... g_akao_xa_tracker;`) is declared BLOCK-SCOPED inside
 * the function that was matched under it. The volatile-vs-plain views of
 * g_akao_spu_xfer_pending, and the -G4 array-form externs used by the re-tuned
 * func_8002D4D8 / func_8002DB90, are block-scoped for the same reason.
 *
 * Only genuinely shared, non-conflicting declarations are hoisted to file
 * scope below: includes, the agreed SfxControl / AkaoDriverFlags / command
 * structs, and the callee prototypes every caller agrees on. akao_driver.h is
 * deliberately NOT included, since it declares its own conflicting
 * g_akao_xa_tracker / AkaoXaTracker; akao.h supplies AkaoChannelState alone.
 */
#include "common.h"
#include "sdk/libspu.h"
#include "akao.h" /* AkaoChannelState only; does NOT declare g_akao_xa_tracker */

/* ---- shared, non-conflicting SFX/driver state (superset layouts) ---- */

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

/** @brief AKAO driver state flags (size 0x0C). */
typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u32 unk8; /* 0x08 - pending driver/SPU hardware update flags */
} AkaoDriverFlags;

/* ---- shared command / parameter structs (no layout conflicts) ---- */

typedef struct
{
    u32 unk0; /* 0x00 */
    u32 unk4; /* 0x04 */
    u32 unk8; /* 0x08 */
} AkaoCmd0C;

typedef struct
{
    s32 unk0;
} AkaoCmdS32;

typedef struct
{
    u32 unk0; /* 0x00 - fade duration in ticks (0 => 1) */
    u16 unk4; /* 0x04 - target pan */
} AkaoXaCmd;

/* Configuration record passed by func_8002DDDC (a0). */
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

/* Streamed SPU buffer; AKAO magic sits at +0x80 (func_8002E5A4 family). */
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

/* ---- shared externs (no conflicts) ---- */
extern SfxControl g_akao_sfx_control;
extern AkaoDriverFlags g_akao_driver_flags;
extern AkaoChannelState* g_akao_seq_channel0;
extern u32 D_8004F76C[];
extern u32 D_8004F754;
extern s16 D_8003D37C[];

/* ---- shared external callee prototypes (all callers agree) ---- */
void akao_release_channels(AkaoChannelState* channel, u32 release_mask);
void akao_sfx_stop_channels(s32 arg0, s32 arg1);
void spu_set_key_off(u32 voice_mask);
void spu_set_key_on(u32 voice_mask);
void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale);
void spu_set_voice_repeat_addr(s32 voice, u32 addr);
void akao_spu_arm_xfer(void);
void akao_spu_write(s32 src_addr, s32 byte_count);
void akao_cmd_e5(s32 a0, s32 a1);

/* ---- forward prototypes for internal callback targets used before defn ---- */
void func_8002D764(void);
void func_8002D7C8(void);
void func_8002D978(void);
void func_8002D9A8(void);
void func_8002D9D8(void);
void func_8002DA08(void);
void func_8002E540(void);
void func_8002E6FC(void);
void func_8002E72C(void);

/**
 * @brief Extended opcode FE 1E: obey the reserved-voice window for this channel.
 *        Clears the channel bit in voice_alloc_low_mask so note-on voice
 *        allocation starts at the reserved base again. Inverse of FE 1D
 *        (akao_seq_op_ignore_voice_reserve).
 * @param channel Unused; present to match the opcode-handler signature.
 * @param channel_mask Bit of the channel being stepped.
 * @see decomp.me (100%)
 */
void akao_seq_op_obey_voice_reserve(AkaoChannelState* channel, s32 channel_mask)
{
    g_akao_seq_channel0->w04.song.voice_alloc_low_mask &= ~channel_mask;
}

/**
 * @brief Opcode 0xE1: set the pitch-jitter depth from one operand byte.
 *        Stores the byte into pitch_scale (0xDA), the depth of the table-driven
 *        pitch perturbation akao_seq_step_opcode applies per note (indexes the
 *        256-entry jitter table D_8003D27C by the free-running modulation tick).
 * @param p Channel stream cursor; advanced past the one depth byte.
 * @note Named by mechanism; the authoring-tool term is unconfirmed.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_pitch_jitter_depth(void *p)
{
    *(u16 *)((char *)p + 0xDA) = *(*(u8 **)p)++;
}

/**
 * @brief Opcode 0xE2: disable pitch jitter by clearing pitch_scale (0xDA).
 * @param p Channel state (operand unused).
 * @see decomp.me (100%)
 */
void akao_seq_op_disable_pitch_jitter(void *p)
{
    *(u16 *)((char *)p + 0xDA) = 0;
}

/**
 * @brief Fallback opcode handler (primary 0xE3..0xFF and unused extended slots):
 *        end the channel by releasing it. Thin wrapper over akao_release_channels.
 * @param arg0 Channel to release.
 * @param arg1 Channel bit-mask to release.
 * @see decomp.me (100%)
 */
void akao_seq_op_finish_channel(AkaoChannelState* arg0, u32 arg1)
{
    akao_release_channels(arg0, arg1);
}

/**
 * @see decomp.me (100%)
 * @note Defined with a K&R empty parameter list (not `(void)`) so that
 *       func_8002E75C can still call it with one argument, as it did when the
 *       two lived in separate translation units. The body takes no parameters,
 *       so its codegen is identical to the `(void)` form.
 */
void func_8002D140()
{
    typedef struct
    {
        u8   pad0[0x08]; /* 0x00 - 0x07 */
        u32  unk8;       /* 0x08 */
        s32  unkC;       /* 0x0C - voice mask */
        s32  unk10;      /* 0x10 */
        u8   pad1[0x0C]; /* 0x14 - 0x1F */
        s32  unk20;      /* 0x20 */
        s32  unk24;      /* 0x24 */
        s32  unk28;      /* 0x28 */
        u8   pad2[0x08]; /* 0x2C - 0x33 */
        s32  unk34;      /* 0x34 */
        s32  unk38;      /* 0x38 */
        s32  unk3C;      /* 0x3C */
        s32  unk40;      /* 0x40 */
        s32  unk44;      /* 0x44 */
        s32  unk48;      /* 0x48 */
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetIRQ(0);
        SpuSetIRQCallback(0);
        spu_set_key_off(g_akao_xa_tracker.unkC);
        g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
        g_akao_xa_tracker.unkC = 0;
        g_akao_driver_flags.unk8 |= 0x100;
    }
}

/**
 * @see decomp.me (100%)
 */
s32 func_8002D1C4(void)
{
    s32 combined;
    s32 count;
    s32 result;
    u32 mask;

    for (;;)
    {
        mask = 0xC00000;
        count = 0xB;
        combined = g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10;
    loop_2:
        if (combined & mask)
        {
            count -= 1;
            mask >>= 1;
            if (count != 0)
            {
                goto loop_2;
            }
        }
        if (count != 0)
        {
            result = count + 0xB;
            return result;
        }
        akao_sfx_stop_channels(0, 0x40000000);
        if (combined == (s32)(g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10))
        {
            return -1;
        }
    }
}

/**
 * @see decomp.me (100%)
 */
void func_8002D254(void)
{
    /* Private XA-tracker view exposing the SPU buffer base at offset 0. */
    typedef struct
    {
        u8 *buf;         /* 0x00 - SPU buffer base */
        u8  pad[0x48];   /* 0x04 - 0x4B */
    } XaTracker;
    extern XaTracker g_akao_xa_tracker;

    u8 *addr = ((XaTracker *)&g_akao_xa_tracker)->buf + 0x800;
    SpuSetTransferStartAddr(0x2100);
    SpuSetTransferCallback(func_8002D764);
    SpuWrite(addr, 0x800);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D29C(void *arg0, s32 arg1, s32 flag)
{
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
    extern XaTracker g_akao_xa_tracker;
    extern s32 g_akao_spu_xfer_pending;

    SeqSrc *hdr;
    s32 slot;
    s32 mask;
    s32 xfer;
    s32 size;
    s32 lead;
    u32 nmask;
    XaTracker *t;
    XaTracker *u;
    XaTracker *p;
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
    p = &g_akao_xa_tracker;

    if (g_akao_xa_tracker.unk8 & 2)
    {
        xfer = (s32)g_akao_xa_tracker.unk0 + hdr->unk14;
    }
    else
    {
        xfer = 0;
    }

    t = &g_akao_xa_tracker;
    p->unk4 = xfer;

    if (t->unk8 & 1)
    {
        u = t;
        if (t->unk8 & 2)
        {
            lead = t->unk14 - ((u32)hdr->unk14 >> 1);
        }
        else
        {
            lead = 0;
        }
        u->unk1C = lead;
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

/**
 * @see decomp.me (100%)
 */
void func_8002D4D8(s32 voice, s32 mode, u32 start, u32 end)
{
    typedef struct
    {
        u8   pad0[0x40];
        s32  unk40;      /* 0x40 - base scale value */
        u8   pad1[0x09]; /* 0x44 - 0x4C */
        u8   unk4D;      /* 0x4D - pan table index */
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;
    /*
     * Declared as unknown-size arrays on purpose: under -G4 a plain `extern s32`
     * (size <= 4) is treated as small-data, so gcc emits a single-register
     * symbolic load that maspsx expands into a same-register `lui/lw` pair. The
     * target uses large-data %hi/%lo addressing with a compiler-split high
     * register (a fresh reg for the %hi part), which the unknown-size array form
     * reproduces. See akao_flush_voice_updates for the same trick.
     */
    extern s32 g_akao_xa_pan_current[];
    extern u32 D_8004F7B8[];
    extern s16 D_8003D47C[];

    void spu_set_voice_pitch(s32 voice, u32 pitch);
    void spu_set_voice_start_addr(s32 voice, u32 addr);
    void spu_set_voice_attack(s32 voice, s32 attack_shift, u32 mode_bits);
    void spu_set_voice_decay_shift(s32 voice, s32 decay_shift);
    void spu_set_voice_sustain_level(s32 voice, s32 sustain_level);
    void spu_set_voice_sustain_mode(s32 voice, s32 sustain_bits, u32 mode_bits);
    void spu_set_voice_release_mode(s32 voice, s32 release_shift, u32 mode_bit);

    s16 vol_l;
    s16 vol_r;

    if (D_8004F754 & 2)
    {
        s32 t = (g_akao_xa_pan_current[0] * D_8003D47C[0]) >> 16;
        vol_r = t;
        vol_l = t;
    }
    else if (mode == 1)
    {
        vol_r = 0;
        vol_l = (u32)g_akao_xa_pan_current[0] >> 1;
    }
    else if (mode == 2)
    {
        vol_l = 0;
        vol_r = (u32)g_akao_xa_pan_current[0] >> 1;
    }
    else if (mode == 3)
    {
        s32 t = (g_akao_xa_pan_current[0] >> 1) << 16;
        vol_r = (t >> 17) + (t >> 18);
        vol_l = vol_r;
    }
    else
    {
        s32 pan = g_akao_xa_tracker.unk4D;
        s32 base = g_akao_xa_tracker.unk40;
        vol_l = (u32)(base * D_8003D37C[pan]) >> 16;
        pan ^= 0xFF;
        vol_r = (u32)(base * D_8003D37C[pan]) >> 16;
    }

    spu_set_voice_volume(voice, vol_l, vol_r, 0);
    spu_set_voice_pitch(voice, D_8004F7B8[0]);
    spu_set_voice_start_addr(voice, start);
    spu_set_voice_repeat_addr(voice, end);
    spu_set_voice_attack(voice, 0, 1);
    spu_set_voice_decay_shift(voice, 0xF);
    spu_set_voice_sustain_level(voice, 0xF);
    spu_set_voice_sustain_mode(voice, 0x7F, 3);
    spu_set_voice_release_mode(voice, 6, 3);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D694(int a0, int a1, void (*cb)())
{
    typedef struct
    {
        u8  *buf;        /* 0x00 */
        u8   pad0[0x08]; /* 0x04 - 0x0B */
        s32  unkC;       /* 0x0C */
        u8   pad1[0x04]; /* 0x10 - 0x13 */
        s32  unk14;      /* 0x14 */
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;
    extern s32 g_akao_spu_xfer_pending;

    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetTransferCallback(0);
        g_akao_spu_xfer_pending = 0;
        if ((u32)g_akao_xa_tracker.unk14 > 0x1000)
        {
            g_akao_xa_tracker.unk14 -= 0x1000;
            g_akao_xa_tracker.buf += a0;
            SpuSetIRQCallback(cb);
        }
        else
        {
            SpuSetIRQCallback(func_8002D140);
            a1 = 0x1030;
            g_akao_xa_tracker.unk14 = 0;
        }
        SpuSetIRQAddr(a1 + 8);
        spu_set_key_on(D_8004F76C[0]);
        SpuSetIRQ(1);
    }
}

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
void func_8002DAA0(AkaoCmdS32 *cmd)
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
void func_8002DB90(AkaoCmdS32 *cmd)
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
    extern s16 D_8003D47C[];

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
            temp_a = (xa->unk40 * D_8003D47C[0]) >> 16;
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

/**
 * @see decomp.me (100%)
 */
void func_8002DDDC(Cfg *cfg, s32 a1, s32 a2, s32 a3)
{
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
    extern XaTrk g_akao_xa_tracker;
    extern XaProgramStaging g_akao_xa_program_staging;

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

/**
 * @see decomp.me (100%)
 */
void func_8002E204(s32* arg0)
{
    func_8002DDDC((Cfg *)arg0[0], arg0[1], arg0[2], arg0[3]);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @see decomp.me (100%)
 */
void func_8002E250(s32* arg0)
{
    func_8002DFA4(arg0[0], arg0[1]);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @see decomp.me (100%)
 */
s32 func_8002E294(s32 *p)
{
    typedef struct
    {
        u8   pad0[0x28]; /* 0x00 - 0x27 */
        s32  unk28;      /* 0x28 */
        u8   pad2C[0x10];/* 0x2C - 0x3B */
        s32  unk3C;      /* 0x3C - frame limit */
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    g_akao_xa_tracker.unk28++;
    (*p)++;
    if ((u32)(g_akao_xa_tracker.unk3C - 1) < (u32)*p)
    {
        *p = 0;
    }
    return *p;
}

/**
 * @see decomp.me (100%)
 */
void func_8002E2E8(void)
{
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
    extern volatile s32 g_akao_spu_xfer_pending;

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
    typedef struct
    {
        s32 unk0;        /* 0x00 */
        u8  pad04[0x04]; /* 0x04 - 0x07 */
        s32 unk8;        /* 0x08 */
        s32 unkC;        /* 0x0C */
        s32 unk10;       /* 0x10 */
        u32 unk14;       /* 0x14 */
    } XaTrk;
    extern XaTrk g_akao_xa_tracker;
    extern s32 g_akao_spu_xfer_pending;

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

/**
 * @see decomp.me (100%)
 */
void func_8002E540(void)
{
    typedef struct
    {
        AkaoBuf *unk0;  /* 0x00 */
        u8  pad0[0x04]; /* 0x04 - 0x07 */
        u32 unk8;       /* 0x08 */
        s32 unkC;       /* 0x0C */
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

    func_8002D4D8(g_akao_xa_tracker.unk10, 1, 0x1100, 0x2100);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, 0x1900, 0x2900);
    func_8002E478(0x2000, 0x2100, func_8002E6FC);
}

/**
 * @see decomp.me (100%)
 */
void func_8002E5A4(s32 arg0, s32 arg1, s32 arg2, void (*arg3)())
{
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
    typedef struct
    {
        u8  pad0[0x08]; /* 0x00 - 0x07 */
        u32 unk8;       /* 0x08 */
        u8  pad1[0x20]; /* 0x0C - 0x2B */
        AkaoBuf *unk2C; /* 0x2C */
        u32 unk30;      /* 0x30 */
    } AkaoXaTracker;
    extern AkaoXaTracker g_akao_xa_tracker;

    AkaoStreamSrc *src = arg0;

    func_8002D140(arg0);
    g_akao_xa_tracker.unk8 = 0x1000000;
    g_akao_xa_tracker.unk2C = src->unk0;
    g_akao_xa_tracker.unk30 = src->unk4;
}
