#include "akao_cmd.h"
#include "akao.h"
#include "psyq/libcd.h"


/**
 * AKAO command parameter buffer. Each command opcode reads its inputs from
 * the first N slots; the layout is opcode-specific and the driver consumes
 * it during akao_send_command.
 *
 * Slot 0 is dual-purpose: most opcodes treat it as a scalar (channel index,
 * sound id, volume), but several wrappers (akao_play_song, akao_register_bank,
 * akao_cmd_e0 opcode 0xE0, akao_cmd_ec opcode 0xEC) store an
 * AkaoSeqHeader-compatible **buffer pointer** there. Typed @c void* to
 * acknowledge that dual use; scalar stores rely on GCC 2.7.2's permissive
 * implicit int→pointer conversion.
 */
extern void *g_akaoCmdParams[];
extern s32 D_8004D400;
extern u8 g_sfx_channels[];

/**
 * Per-tick scratch state for the AKAO bank-streaming uploader.
 *
 * Primed on the first tick of a streaming upload from the AkaoBankHeader at
 * the head of the source buffer; each subsequent call to
 * akao_streaming_upload_tick consumes some bytes from the source and shrinks
 * the two `*_remaining` counters. When both reach zero (and the external
 * latch @c D_8004F828 is also clear), the streaming-pending bit in
 * @c g_akao_driver_flags is cleared.
 */
typedef struct
{
    void* articulation_dst;     /* 0x00: current dst into the driver's
                                          articulation slot table
                                          (D_8004C340 + bank_id * 0x10),
                                          advances as bytes are copied      */
    u32 spu_addr;               /* 0x04: current SPU upload address — seeded
                                          from AkaoBankHeader.spu_dest_addr,
                                          advances as samples are written;
                                          a value of 0 marks "first tick"   */
    u32 sample_remaining;       /* 0x08: bytes of sample data still to send
                                          to the SPU                        */
    u32 articulation_remaining; /* 0x0C: bytes of articulation data still to
                                          copy into the driver's slot table */
} AkaoStreamingState;

typedef struct {
    u8   pad0[0x08];           /* 0x00 - 0x07 */
    u32  unk8;                 /* 0x08 - from func_80023334 */
    u8   pad1[0x14];           /* 0x0C - 0x1F (20 bytes) */
    s32  unk20;                /* 0x20 - from func_800232A8 */
    s32  unk24;                /* 0x24 - used in both functions */
    s32  unk28;                /* 0x28 - from func_800232A8 */
    u8   pad2[0x08];           /* 0x2C - 0x33 (8 bytes) */
    s32  unk34;                /* 0x34 - from func_800232A8 */
    s32  unk38;                /* 0x38 - used in both functions */
    s32  unk3C;                /* 0x3C - from func_800232A8 */
} AkaoXaTracker;  /* Total size: 0x40 (64 bytes) */

extern s32 D_8004F794;
extern s32 D_8004C170;
extern u32 D_8004C150;
extern AkaoChannelState* g_akao_seq_channel0;
extern CdlATV g_akao_cdmix;
extern s32 D_8004F754;
extern u8 D_8004C340[];
extern s32 D_8004D388;
extern s32 D_8004D38C;
extern s32 D_8004D390;
extern s32 D_8004D394;
extern s32 D_8004D398;
extern s32 D_8004D39C;
extern s32 g_akao_driver_flags;
extern s32 g_akao_spu_xfer_pending;
extern s32 D_8004F824;
extern s32 D_8004F828;
extern AkaoStreamingState g_akao_streaming_state;
extern AkaoBankHeader g_akao_bank_staging;

extern AkaoXaTracker g_akao_xa_tracker;

#define AKAO_CHANNEL_STATE (*(AkaoChannelState**)0x8003EC5C)

extern s32 akao_submit(AkaoSeqHeader* sequenceData, s32 waitForCompletion);

s32 FUN_80021fbc(void);
s32 func_80021FDC(void);
s32 akao_register_bank(AkaoSeqHeader* bank);
void akao_play_song(s32 seqData);
void akao_stop_song(s32 arg0);
void akao_cmd_40(void);
void akao_cmd_14(s32 arg0, s32 arg1);
s32 akao_cmd_19_c0(s32 arg0, s32 arg1);
void akao_cmd_12(s32 arg0, s32 arg1);
void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 akao_play_sfx_from_buffer(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void akao_cmd_21(s32 arg0, s32 arg1);
void akao_stop_sfx_by_id(s32 arg0);
s32 func_800222A8(void);
s32 func_80022310(s32 arg0);
void akao_set_paused(s32 arg0);
void akao_cmd_90(s32 arg0);
void akao_cmd_92(s32 arg0);
void akao_cmd_99_9b_9d_9f(u32 param_1);
void akao_cmd_98_9a_9c_9e(u32 arg0);

/**
 * Central dispatcher for the AKAO sound driver. Each high-level wrapper
 * (akao_play_song, akao_stop_song, akao_play_sfx, etc.) writes its inputs
 * into g_akaoCmdParams and then invokes this function with a one-byte
 * command opcode. Known opcodes used in this codebase: 0x10 play song,
 * 0x11 stop song, 0x12, 0x14, 0x19, 0x20 play SFX, 0x21, 0x24, 0x30, 0x40,
 * 0x80/0x81, 0x90/0x92, 0xA0/0xA1/0xA8/0xA9, 0xC0/0xC1, 0xF0/0xF1.
 *
 * Declared @c void here even though some call sites read its return value;
 * the original GCC 2.7.2 build tolerates the mixed declaration (callers rely
 * on the $v0 register convention).
 */
extern s32 akao_send_command(s32 opcode);



/**
 * @brief Public init entry — wraps akao_driver_init and returns 0.
 * @see https://decomp.me/scratch/hDNyF (100%)
 */
s32 FUN_80021fbc(void)
{
    akao_driver_init();
    return 0;
}

/**
 * @brief Public shutdown entry — wraps akao_driver_shutdown and returns 0.
 * @see https://decomp.me/scratch/z7ZEh (100%)
 */
s32 func_80021FDC(void)
{
    akao_driver_shutdown();
    return 0;
}

/**
 * @brief Registers an AKAO instrument/sample bank with the audio driver.
 *
 * Validates the 'AKAO' magic at the start of @p bankBase via akao_check_magic;
 * on success, hands the payload (after the 16-byte AKAO header) to the driver
 * entry point func_80023BB8, which records the bank as the active sample source.
 *
 * @param bankBase  Address of an AKAO-tagged instrument bank in main RAM.
 *                  The first 4 bytes must be "AKAO" (0x4F414B41 little-endian).
 *                  In TITLE this points to 0x8013C000 after EFFECT.SET is split.
 *
 * @return 0 if the magic matched and the bank was registered; otherwise the
 *         non-zero delta (*bankBase + 0xB0BEB4BF) that akao_check_magic returned.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/0q180
 */
s32 akao_register_bank(AkaoSeqHeader* bank)
{
    s32 temp_v0;

    temp_v0 = akao_check_magic((s32*)bank);
    if (temp_v0 == 0)
    {
        func_80023BB8((s32)bank + sizeof(AkaoSeqHeader));
    }
    return temp_v0;
}

/**
 * @brief AKAO command 0x10 — start playback of a sequence (song).
 *
 * Loads @p seqData into the AKAO command parameter buffer and dispatches the
 * "play song" command to the audio driver. The driver picks up the buffer
 * pointer from g_akaoCmdParams[0] when it processes the command.
 *
 * @param seqData  Pointer to a loaded AKAO-tagged sequence buffer (e.g.
 *                 @c &D_8003ECA0 in TITLE, @c &D_8005D088 in CHECKPS).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iVOOb
 */
void akao_play_song(s32 seqData)
{
    g_akaoCmdParams[0] = (void*)(seqData);
    akao_send_command(0x10);
}

/**
 * @brief AKAO command 0x11 — stop the currently-playing sequence.
 *
 * Pushes @p arg0 into the AKAO command parameter buffer and dispatches the
 * "stop song" command. Callers in TITLE/CHECKPS pass 0; the precise meaning
 * of non-zero values (likely a fade-out duration or flag) is not yet known.
 *
 * @param arg0  Stop-modifier parameter; observed value is 0 in all callers.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/9M4hF
 */
void akao_stop_song(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    akao_send_command(0x11);
}

/**
 * @brief AKAO command 0x40 — global stop / driver halt.
 *
 * Zero-argument command. Observed callers in cdrom.c and others use this to
 * silence everything (sequences and active SFX) when entering loading screens
 * or other audio-quiescent states.
 *
 * @see https://decomp.me/scratch/4GVez (100%)
 */
void akao_cmd_40(void)
{
    akao_send_command(0x40);
}

/**
 * @brief AKAO command 0x14 — three args, third slot forced 0; semantics TBD.
 *
 * @see https://decomp.me/scratch/c2C3m (100%)
 */
void akao_cmd_14(s32 arg0, s32 arg1)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    g_akaoCmdParams[1] = (void*)(arg1);
    g_akaoCmdParams[2] = (void*)(0);
    akao_send_command(0x14);
}

/**
 * @brief Combo: dispatch AKAO command 0x19 (a) then 0xC0 (b masked to 7 bits).
 *
 * @see https://decomp.me/scratch/d6xXt (100%)
 */
s32 akao_cmd_19_c0(s32 arg0, s32 arg1)
{
    s32 temp_v0;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_v0 = akao_send_command(0x19);
    g_akaoCmdParams[0] = (void*)((s32)(arg1 & 0x7F));
    g_akaoCmdParams[3] = (void*)(0);
    akao_send_command(0xC0);
    return temp_v0;
}

/**
 * @brief AKAO command 0x12 — two unmasked args; semantics TBD.
 *
 * @see https://decomp.me/scratch/jigab (100%)
 */
void akao_cmd_12(s32 arg0, s32 arg1)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    g_akaoCmdParams[1] = (void*)(arg1);
    akao_send_command(0x12);
}

/**
 * @brief AKAO command 0x20 — play a sound effect.
 *
 * Packs four caller-supplied values into the AKAO command parameter buffer,
 * each masked to the bit-width the driver expects, then dispatches the
 * "play SFX" command. The mask widths suggest:
 *   arg0 (10 bits) — sound id / SFX index
 *   arg1 (24 bits) — wider opaque parameter (possibly pitch/frequency)
 *   arg2 ( 8 bits) — byte-sized parameter (possibly pan)
 *   arg3 ( 7 bits) — volume (0–127)
 * Caller in TITLE: PlayTitleSfx(soundId, _, arg1, 0x7F).
 *
 * @param arg0  Sound id (lower 10 bits used).
 * @param arg1  24-bit packed parameter.
 * @param arg2  8-bit parameter.
 * @param arg3  Volume (0–127).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/9AZZL
 */
void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_a3;

    g_akaoCmdParams[0] = (void*)((s32)(arg0 & 0x3FF));
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0xFF;
    temp_a3 = arg3 & 0x7F;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    g_akaoCmdParams[3] = (void*)(temp_a3);
    akao_send_command(0x20);
};

/**
 * @brief AKAO command 0x24 — play SFX from a caller-supplied AKAO buffer (magic-checked); same arg shape as
 * akao_play_sfx (24/8/7-bit).
 *
 * @see https://decomp.me/scratch/FFGei (100%)
 */
s32 akao_play_sfx_from_buffer(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 result = akao_check_magic(arg0);

    if (result != 0)
    {
        return result;
    }

    g_akaoCmdParams[0] = (void*)(arg0);
    g_akaoCmdParams[1] = (void*)(arg1 & 0xFFFFFF);
    g_akaoCmdParams[2] = (void*)(arg2 & 0xFF);
    g_akaoCmdParams[3] = (void*)(arg3 & 0x7F);
    akao_send_command(0x24);

    return arg0;
}

/**
 * @brief AKAO command 0x21 — (id, p24) sound id plus 24-bit param.
 *
 * @see https://decomp.me/scratch/lu9nS (100%)
 */
void akao_cmd_21(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFFFFFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    akao_send_command(0x21);
}

/**
 * @brief AKAO command 0x30 — stop SFX whose 10-bit sound id matches @p arg0.
 *
 * @see https://decomp.me/scratch/0mLzI (100%)
 */
void akao_stop_sfx_by_id(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0 & 0x3FF);
    akao_send_command(0x30);
}

/**
 * @brief Scans active SFX channels and ORs together their offset-0x28 fields.
 *
 * Iterates over the 12 SFX-channel slots in @c g_sfx_channels (each 0x118 bytes),
 * gated by the bitmap in @c D_8004D400 (one bit per channel starting at
 * 0x1000); returns the bitwise-OR of the 32-bit value at offset 0x28 of every
 * active slot, masked to 24 bits.
 *
 * @see https://decomp.me/scratch/yZloM (100%)
 */
s32 func_800222A8(void)
{
    s32 bits;
    unsigned char* ptr;
    s32 new_var;
    s32 acc;
    unsigned int mask;
    new_var = D_8004D400;
    bits = new_var;
    acc = bits == 0;
    if (acc)
    {
        return 0;
    }
    ptr = g_sfx_channels;
    acc = 0;
    mask = 0x1000;
    do
    {
        if (bits & mask)
        {
            acc |= *((s32*)(ptr + 0x28));
        }
        mask <<= 1;
        ptr += 0x118;
    } while (mask & 0xFFFFFF);
    return acc & 0xFFFFFF;
}

/**
 * @brief Returns 1 if any active SFX channel's offset-0x28 field equals @p arg0.
 *
 * Same iteration shape as @c func_800222A8 over @c g_sfx_channels / @c D_8004D400,
 * but compares each active channel's offset-0x28 value to @p arg0; returns
 * 1 on first match, 0 otherwise.
 *
 * @param arg0  Sound id / handle to look for.
 * @return 1 if a matching active channel exists, 0 otherwise.
 *
 * @see https://decomp.me/scratch/OvqYq (100%)
 */
s32 func_80022310(s32 arg0)
{
    s32 new_var;
    s32 bits;
    unsigned char* ptr;
    unsigned int mask;
    if (arg0 == 0)
    {
        return 0;
    }
    bits = D_8004D400;
    if (bits == 0)
    {
        return 0;
    }
    ptr = g_sfx_channels;
    mask = 0x1000;
    do
    {
        if ((new_var = bits) & mask)
        {
            if (arg0 == (*((s32*)(ptr + 0x28))))
            {
                return 1;
            }
        }
        mask <<= 1;
        ptr += 0x118;
    } while (mask & 0xFFFFFF);
    return 0;
}

/**
 * @brief AKAO command 0x80 / 0x81 — pause or resume the active sequence.
 *
 * Picks opcode 0x81 when @p arg0 == 1 (resume) and 0x80 otherwise (pause),
 * then dispatches with no parameter buffer payload. Used by TITLE.OVL to
 * pause music while the title screen is dismissed.
 *
 * @param arg0  1 = resume (0x81); any other value = pause (0x80).
 *
 * @note The local pointer @c new_var2 = &new_var is a register-allocation
 *       artifact required for asm matching; do not remove it. Likewise the
 *       ternary form is load-bearing — see project memory
 *       feedback_no_ternary_rewrites.
 *
 * @see https://decomp.me/scratch/9qTjH (100%)
 */
void akao_set_paused(s32 arg0)
{
    int* new_var2;
    int new_var;
    new_var = (arg0 == 1) ? (0x81) : (0x80);
    new_var2 = &new_var;
    akao_send_command(new_var);
}

/**
 * @brief AKAO command 0x90 — single unmasked arg; semantics TBD.
 *
 * @see https://decomp.me/scratch/x94md (100%)
 */
void akao_cmd_90(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    akao_send_command(0x90);
}

/**
 * @brief AKAO command 0x92 — single unmasked arg; semantics TBD.
 *
 * @see https://decomp.me/scratch/y9TAf (100%)
 */
void akao_cmd_92(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    akao_send_command(0x92);
}

/**
 * @brief Dispatch one of AKAO commands 0x99/0x9B/0x9D/0x9F (zero-arg) selected by @p param_1 (1/2/3/default).
 *
 * @see https://decomp.me/scratch/qqSuG (100%)
 */
void akao_cmd_99_9b_9d_9f(u32 param_1)
{
    s32 var_a0;

    switch (param_1)
    {
    case 1:
        var_a0 = 0x9B;
        break;
    case 2:
        var_a0 = 0x9D;
        break;
    case 3:
        var_a0 = 0x9F;
        break;
    default:
        var_a0 = 0x99;
        break;
    }

    akao_send_command(var_a0);
}

/**
 * @brief Dispatch one of AKAO commands 0x98/0x9A/0x9C/0x9E (zero-arg) selected by @p arg0 (1/2/3/default).
 *
 * @see https://decomp.me/scratch/iREFc (100%)
 */
void akao_cmd_98_9a_9c_9e(u32 arg0)
{
    s32 var_a0;

    switch (arg0)
    {
    case 1:
        var_a0 = 0x9A;
        break;
    case 2:
        var_a0 = 0x9C;
        break;
    case 3:
        var_a0 = 0x9E;
        break;
    default:
        var_a0 = 0x98;
        break;
    }

    akao_send_command(var_a0);
}

/**
 * @brief AKAO command 0xA8 — global counterpart of 0xA0; takes a 7-bit value.
 *
 * @see https://decomp.me/scratch/VTGCB (100%)
 */
s32 akao_cmd_a8(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0 & 0x7F);
    return akao_send_command(0xA8);
}

/**
 * @brief AKAO command 0xA9 — global counterpart of 0xA1; (a, 7-bit value).
 *
 * @see https://decomp.me/scratch/03hNO (100%)
 */
void akao_cmd_a9(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0x7F;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    akao_send_command(0xA9);
}

/**
 * @brief AKAO command 0xA0 — per-channel: (channel, 24-bit fade duration, 7-bit target value).
 *
 * @see https://decomp.me/scratch/C8UTP (100%)
 */
void akao_cmd_a0(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0x7F;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    akao_send_command(0xA0);
}

/**
 * @brief AKAO command 0xA1 — per-channel: (channel, 24-bit fade duration, p, 7-bit target value).
 *
 * @see https://decomp.me/scratch/xMNn0 (100%)
 */
void akao_cmd_a1(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a3;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a3 = arg3 & 0x7F;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(arg2);
    g_akaoCmdParams[3] = (void*)(temp_a3);
    akao_send_command(0xA1);
}

/**
 * @brief AKAO command 0xAA — global counterpart of 0xA2; takes an 8-bit value.
 *
 * @see https://decomp.me/scratch/AuyLX (100%)
 */
void akao_cmd_aa(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0 & 0xFF);
    akao_send_command(0xAA);
}

/**
 * @brief AKAO command 0xAB — global counterpart of 0xA3; (a, 8-bit value).
 *
 * @see https://decomp.me/scratch/IaBX9 (100%)
 */
void akao_cmd_ab(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    akao_send_command(0xAB);
}

/**
 * @brief AKAO command 0xA2 — per-channel: (channel, 24-bit fade duration, 8-bit target value).
 *
 * @see https://decomp.me/scratch/LhoLV (100%)
 */
void akao_cmd_a2(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    akao_send_command(0xA2);
}

/**
 * @brief AKAO command 0xA3 — per-channel: (channel, 24-bit fade duration, p, 8-bit target value).
 *
 * @see https://decomp.me/scratch/Al5YT (100%)
 */
void akao_cmd_a3(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a3;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a3 = arg3 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(arg2);
    g_akaoCmdParams[3] = (void*)(temp_a3);
    akao_send_command(0xA3);
}

/**
 * @brief AKAO command 0xAC — global counterpart of 0xA4; takes an 8-bit value.
 *
 * @see https://decomp.me/scratch/e4D90 (100%)
 */
void akao_cmd_ac(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0 & 0xFF);
    akao_send_command(0xAC);
}

/**
 * @brief AKAO command 0xAD — global counterpart of 0xA5; (a, 8-bit value).
 *
 * @see https://decomp.me/scratch/Fw2d9 (100%)
 */
void akao_cmd_ad(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    akao_send_command(0xAD);
}

/**
 * @brief AKAO command 0xA4 — per-channel: (channel, 24-bit fade duration, 8-bit target value).
 *
 * @see https://decomp.me/scratch/vHMVZ (100%)
 */
s32 akao_cmd_a4(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    return akao_send_command(0xA4);
}

/**
 * @brief AKAO command 0xA5 — per-channel: (channel, 24-bit fade duration, p, 8-bit target value).
 *
 * @see https://decomp.me/scratch/exTVG (100%)
 */
s32 akao_cmd_a5(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a3;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a3 = arg3 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(arg2);
    g_akaoCmdParams[3] = (void*)(temp_a3);
    return akao_send_command(0xA5);
}

/**
 * @brief AKAO command 0xC0 — (a, 7-bit value).
 *
 * @see https://decomp.me/scratch/QPqUd (100%)
 */
s32 akao_cmd_c0(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0x7F;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    return akao_send_command(0xC0);
}

/**
 * @brief AKAO command 0xC1 — 0xC0 with extra middle parameter: (a, b, 7-bit value).
 *
 * @see https://decomp.me/scratch/cSIwP (100%)
 */
s32 akao_cmd_c1(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a2;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a2 = arg2 & 0x7F;
    g_akaoCmdParams[1] = (void*)(arg1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    return akao_send_command(0xC1);
}

/**
 * @brief AKAO command 0xC2 — 0xC0 with two trailing 7-bit values: (a, b, 7-bit, 7-bit).
 *
 * @see https://decomp.me/scratch/PbMJC (100%)
 */
s32 akao_cmd_c2(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a2;
    s32 temp_a3;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a2 = arg2 & 0x7F;
    temp_a3 = arg3 & 0x7F;
    g_akaoCmdParams[1] = (void*)(arg1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    g_akaoCmdParams[3] = (void*)(temp_a3);
    return akao_send_command(0xC2);
}

/**
 * @brief AKAO command 0xC8 — single unmasked arg; semantics TBD.
 *
 * @see https://decomp.me/scratch/BeJR1 (100%)
 */
s32 akao_cmd_c8(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    return akao_send_command(0xC8);
}

/**
 * @brief AKAO command 0xC9 — two unmasked args; semantics TBD.
 *
 * @see https://decomp.me/scratch/yo40G (100%)
 */
s32 akao_cmd_c9(s32 arg0, s32 arg1)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    g_akaoCmdParams[1] = (void*)(arg1);
    return akao_send_command(0xC9);
}

/**
 * @brief AKAO command 0xCA — three unmasked args; semantics TBD.
 *
 * @see https://decomp.me/scratch/pLMBi (100%)
 */
s32 akao_cmd_ca(s32 arg0, s32 arg1, s32 arg2)
{
    g_akaoCmdParams[0] = (void*)(arg0);
    g_akaoCmdParams[1] = (void*)(arg1);
    g_akaoCmdParams[2] = (void*)(arg2);
    return akao_send_command(0xCA);
}

/**
 * @brief AKAO command 0xD0 — (8-bit value).
 *
 * @see https://decomp.me/scratch/klUxi (100%)
 */
s32 akao_cmd_d0(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0 & 0xFF);
    return akao_send_command(0xD0);
}

/**
 * @brief AKAO command 0xD1 — (a, 8-bit value).
 *
 * @see https://decomp.me/scratch/XXHwt (100%)
 */
s32 akao_cmd_d1(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    return akao_send_command(0xD1);
}

/**
 * @brief AKAO command 0xD2 — (a, 8-bit value, 8-bit value).
 *
 * @see https://decomp.me/scratch/074UT (100%)
 */
s32 akao_cmd_d2(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    return akao_send_command(0xD2);
}

/**
 * @brief AKAO command 0xD4 — (8-bit value).
 *
 * @see https://decomp.me/scratch/yJdLv (100%)
 */
s32 akao_cmd_d4(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0 & 0xFF);
    return akao_send_command(0xD4);
}

/**
 * @brief AKAO command 0xD5 — (a, 8-bit value).
 *
 * @see https://decomp.me/scratch/u6Eys (100%)
 */
s32 akao_cmd_d5(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    return akao_send_command(0xD5);
}

/**
 * @brief AKAO command 0xD6 — (a, 8-bit value, 8-bit value).
 *
 * @see https://decomp.me/scratch/ITNFU (100%)
 */
void akao_cmd_d6(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    akao_send_command(0xD6);
}

/**
 * @brief AKAO command 0xD8 — (8-bit value).
 *
 * @see https://decomp.me/scratch/JS2nD (100%)
 */
s32 akao_cmd_d8(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)(arg0 & 0xFF);
    return akao_send_command(0xD8);
}

/**
 * @brief AKAO command 0xD9 — (a, 8-bit value).
 *
 * @see https://decomp.me/scratch/YD6rZ (100%)
 */
s32 akao_cmd_d9(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    return akao_send_command(0xD9);
}

/**
 * @brief AKAO command 0xDA — (a, 8-bit value, 8-bit value).
 *
 * @see https://decomp.me/scratch/jzW0l (100%)
 */
s32 akao_cmd_da(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = arg1 & 0xFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    g_akaoCmdParams[2] = (void*)(temp_a2);
    return akao_send_command(0xDA);
}

/**
 * @brief AKAO command 0xF0 — zero-arg query; return value consumed by caller.
 *
 * @see https://decomp.me/scratch/dgbnE (100%)
 */
s32 akao_cmd_f0(void)
{
    return akao_send_command(0xF0);
}

/**
 * @brief AKAO command 0xF1 — zero-arg query; return value consumed by caller.
 *
 * @see https://decomp.me/scratch/IMYAL (100%)
 */
s32 akao_cmd_f1(void)
{
    return akao_send_command(0xF1);
}

/**
 * @brief Submits an AKAO sequence to the audio driver and spins until accepted.
 *
 * Clears bit 0 of the driver status word, then repeatedly calls akao_submit
 * until it stops returning the "busy" sentinel. Used to hand a freshly-loaded
 * AKAO sequence (BGM or SFX program) to the driver and block until the SPU
 * transfer window opens and the data is consumed.
 *
 * @param sequenceData       Pointer to an AKAO-tagged sequence buffer in main RAM.
 * @param waitForCompletion  When non-zero, akao_submit further blocks inside the
 *                           driver until the SPU DMA completes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Mz7yX
 */
void akao_play_sequence_blocking(AkaoSeqHeader* sequenceData, s32 waitForCompletion)
{
    g_akao_driver_flags &= ~1;
    while (akao_submit(sequenceData, waitForCompletion) == 1);
}

/**
 * @brief Returns the current SPU/AKAO transfer state latch (g_akao_spu_xfer_pending).
 *
 * @see https://decomp.me/scratch/ecQHb (100%)
 */
s32 akao_get_xfer_state(void)
{
    return g_akao_spu_xfer_pending;
}

/**
 * @brief Clears the streaming-upload state (D_8004F824) and asserts the transfer-pending flag.
 *
 * @see https://decomp.me/scratch/qBE70 (100%)
 */
s32 akao_reset_xfer_state(void)
{
    D_8004F824 = 0;
    g_akao_driver_flags |= 1;
    return 0;
}

/**
 * @brief Advances one tick of the AKAO bank-streaming upload state machine.
 *
 * Each tick is a single bounded copy step. The driver loads an instrument
 * bank in three stages — header, articulation table, sample blob — and the
 * caller drives this function repeatedly with whatever fresh bytes it has
 * read from disk so far.
 *
 * Stage 1 (first tick — @c g_akao_streaming_state.spu_addr == 0):
 *   - Magic-check @p src. On failure, zero the residual counters so the
 *     subsequent stages all short-circuit and the streaming-pending bit
 *     gets cleared at the bottom.
 *   - Copy the 0x40-byte AkaoBankHeader into @c g_akao_bank_staging and
 *     advance @p src/@p avail past it.
 *   - Seed @c g_akao_streaming_state from the staged header:
 *       @c spu_addr               ← spu_dest_addr
 *       @c sample_remaining       ← sample_size
 *       @c articulation_dst       ← &D_8004C340[bank_id * 0x10]
 *       @c articulation_remaining ← articulation_count * 0x10
 *
 * Stage 2 (articulation copy):
 *   - memcpy up to @c articulation_remaining bytes of the source into the
 *     driver's articulation slot, advancing both pointers and shrinking
 *     the residual.
 *   - When the residual hits zero, rebase the articulation entries onto
 *     the SPU base via @c akao_relocate_articulations.
 *
 * Stage 3 (sample upload):
 *   - SpuSetTransferStartAddr(@c spu_addr), then akao_spu_write the source.
 *   - Advance @c spu_addr by the chunk size and shrink @c sample_remaining.
 *   - If @p wait_for_spu is non-zero, block on akao_spu_wait.
 *
 * Once everything is consumed (and the @c D_8004F828 latch is also clear),
 * the streaming-pending bit in @c g_akao_driver_flags is cleared.
 *
 * @param src          Source byte pointer in main RAM. Starts at the AKAO
 *                     header on the first tick and advances through the
 *                     articulation and sample regions across subsequent
 *                     ticks.
 * @param avail        Number of fresh bytes available to consume this tick.
 * @param wait_for_spu Non-zero ⇒ block on @c akao_spu_wait after the SPU
 *                     write completes.
 *
 * @return @c D_8004F828 (the streaming-status latch read by callers).
 *
 * @note The @c akao_spu_write call passes @p avail as the byte count rather
 *       than the clamped @c sample_chunk; this mirrors the original asm
 *       literally and is the suspected reason this scratch sits at 98.59%.
 *
 * @see decomp.me (98.59%) https://decomp.me/scratch/0IPqT
 */
s32 akao_streaming_upload_tick(s32 src, u32 avail, s32 wait_for_spu)
{
    s32 advance;          /* word-aligned bytes consumed from articulation */
    u32 chunk;            /* clamped articulation chunk size               */
    u32 sample_chunk;     /* clamped sample chunk size                     */
    s32* new_var;         /* load-bearing temp for codegen                 */
    void* arti_slot;      /* base of this bank's articulation slot         */

    if ((g_akao_driver_flags & 1) == 0)
    {
        return D_8004F828;
    }
    if (g_akao_streaming_state.spu_addr == 0)
    {
        if (akao_check_magic(src) == 0)
        {
            func_80029A0C(*(new_var = &src), &g_akao_bank_staging, 0x40U);
            src += 0x40;
            avail -= 0x40;
            g_akao_streaming_state.spu_addr = (s32)g_akao_bank_staging.spu_dest_addr;
            g_akao_streaming_state.sample_remaining = (u32)g_akao_bank_staging.sample_size;
            g_akao_streaming_state.articulation_dst = (void*)((g_akao_bank_staging.bank_id * 0x10) + ((u32)(&D_8004C340)));
            g_akao_streaming_state.articulation_remaining = (u32)(g_akao_bank_staging.articulation_count * 0x10);
        }
        else
        {
            avail = 0;
            g_akao_streaming_state.sample_remaining = 0U;
            g_akao_streaming_state.articulation_remaining = 0U;
        }
    }
    if (g_akao_streaming_state.articulation_remaining != 0)
    {
        chunk = g_akao_streaming_state.articulation_remaining;
        if (avail != 0)
        {
            if (chunk >= avail)
            {
                chunk = avail;
            }
            func_80029A0C(src, g_akao_streaming_state.articulation_dst, chunk);
            advance = (chunk >> 2) * 4;
            src += advance;
            avail -= chunk;
            g_akao_streaming_state.articulation_dst = (void*)(((u32)g_akao_streaming_state.articulation_dst) + advance);
            g_akao_streaming_state.articulation_remaining -= chunk;
            if (g_akao_streaming_state.articulation_remaining == 0)
            {
                arti_slot = (void*)((g_akao_bank_staging.bank_id * 0x10) + ((u32)(&D_8004C340)));
                akao_relocate_articulations(arti_slot, arti_slot, g_akao_bank_staging.spu_dest_addr,
                                            g_akao_bank_staging.articulation_count);
            }
        }
    }
    if (avail != 0)
    {
        if (g_akao_streaming_state.sample_remaining != 0)
        {
            sample_chunk = g_akao_streaming_state.sample_remaining;
            if (g_akao_streaming_state.sample_remaining >= avail)
            {
                sample_chunk = avail;
            }
            chunk = sample_chunk;
            SpuSetTransferStartAddr(g_akao_streaming_state.spu_addr);
            akao_spu_write(src, avail);
            g_akao_streaming_state.spu_addr += sample_chunk;
            g_akao_streaming_state.sample_remaining -= chunk;
            if (wait_for_spu != 0)
            {
                akao_spu_wait();
            }
        }
        else
        {
            goto block_18;
        }
    }
    if (D_8004F828 == 0)
    {
    block_18:
        g_akao_driver_flags &= ~1;
    }
    return D_8004F828;
}

/**
 * @brief Wrapper: blocks via akao_play_sequence_blocking and returns 0.
 *
 * @see https://decomp.me/scratch/0f3IK (100%)
 */
s32 akao_load_sequence(AkaoSeqHeader* sequenceData, s32 waitForCompletion)
{
    akao_play_sequence_blocking(sequenceData, waitForCompletion);
    return 0;
}

/**
 * @brief Routes an AKAO bank to one of six SPU base/slot pairs by @p arg1, records the bank id, and uploads.
 *
 * @see https://decomp.me/scratch/FWcdy (100%)
 */
s32 akao_upload_bank_slot(void* arg0, s32 arg1, s32 arg2)
{
    s32 var_a2;
    s32 var_a3_2;
    s32* var_t0;
    u32 var_a3;
    void* tmp = arg0;
    var_a3 = 0;
    var_t0 = &D_8004D388;
    do
    {
        if ((*var_t0) == ((s32*)tmp)[1])
        {
            var_a3++;
            var_a3--;
            *var_t0 = 0;
        }
        var_a3 += 1;
        var_t0 += 1;
    }

    while (var_a3 < 6U);
    switch (arg1)
    {
    case 1:
        var_a3_2 = 0x47900;
        var_a2 = 0x90;
        D_8004D38C = ((s32*)tmp)[1];
        break;

    case 2:
        var_a3_2 = 0x4C100;
        var_a2 = 0xA0;
        D_8004D390 = ((s32*)tmp)[1];
        break;

    case 3:
        var_a3_2 = 0x50900;
        var_a2 = 0xB0;
        D_8004D394 = ((s32*)tmp)[1];
        break;

    case 4:
        var_a3_2 = 0x55100;
        var_a2 = 0xC0;
        D_8004D398 = ((s32*)tmp)[1];
        break;

    case 5:
        var_a3_2 = 0x59900;
        var_a2 = 0xD0;
        D_8004D39C = ((s32*)tmp)[1];
        break;

    default:
        var_a3_2 = 0x43100;
        var_a2 = 0x80;
        D_8004D388 = ((s32*)tmp)[1];
        break;
    }

    akao_upload_bank(arg0, arg2, var_a2, var_a3_2);
    return 0;
}

/**
 * @brief Wrapper: forwards @p arg1 unchanged to akao_upload_bank_slot
 *        (selects bank slots 0..5 directly).
 * @see https://decomp.me/scratch/sa1fh (100%)
 */
s32 func_80022ED8(void* arg0, s32 arg1, s32 arg2)
{
    akao_upload_bank_slot(arg0, arg1, arg2);
    return 0;
}

/**
 * @brief Wrapper: biases @p arg1 by 3 before calling akao_upload_bank_slot
 *        (selects the second-half bank slots 3..8).
 * @see https://decomp.me/scratch/PnDWc (100%)
 */
s32 func_80022EF8(void* arg0, s32 arg1, s32 arg2)
{
    akao_upload_bank_slot(arg0, arg1 + 3, arg2);
    return 0;
}

/**
 * @brief Programs the CD/XA mix volume registers (@c CdMix on @c g_akao_cdmix).
 *
 * If bit 1 of @c D_8004F754 is set, all four CdlATV slots get
 * @c (arg0 * 0xB570) >> 0x11 — a 16-bit-fixed-point scale of @p arg0 across
 * a stereo pair. Otherwise only the two "main" slots get @p arg0 and the
 * "side" slots are zeroed. @p arg1 is ignored here but participates in the
 * larger XA-streaming setup at the callers.
 *
 * @param arg0  Target CD volume (0–127 expected).
 * @param arg1  Reserved / unused at this call site.
 *
 * @see https://decomp.me/scratch/hcfmi (100%)
 */
s32 akao_xa_setup_panning(s32 arg0, void* arg1)
{
    s32 new_var;
    u32 temp_v0;
    s32 val = arg0;
    new_var = val;
    if (D_8004F754 & 2)
    {

        g_akao_cdmix.val3 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
        g_akao_cdmix.val1 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
        g_akao_cdmix.val2 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
        g_akao_cdmix.val0 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
    }
    else
    {
        g_akao_cdmix.val2 = (u_char)new_var;
        g_akao_cdmix.val0 = (u_char)new_var;
        g_akao_cdmix.val3 = 0;
        g_akao_cdmix.val1 = 0;
    }
    CdMix(&g_akao_cdmix);
    return 0;
}

/**
 * @brief AKAO command 0xE0 — magic-checks @p arg0 (AKAO buffer) then dispatches with (buf*, 16-bit packed, c).
 *
 * @see https://decomp.me/scratch/vw9QX (100%)
 */
void akao_cmd_e0(s32 arg0, s32 arg1, s32 arg2)
{
    if (akao_check_magic(arg0) == 0)
    {
        g_akaoCmdParams[0] = (void*)(arg0);
        g_akaoCmdParams[1] = (void*)((s32)((arg1 & 0xFF) << 8));
        g_akaoCmdParams[2] = (void*)(arg2);
        akao_send_command(0xE0);
    }
}

/**
 * @brief AKAO command 0xE2 — zero-arg.
 *
 * @see https://decomp.me/scratch/kd4bK (100%)
 */
s32 akao_cmd_e2(void)
{
    return akao_send_command(0xE2);
}

/**
 * @brief AKAO command 0xE4 — set the CD/XA channel mix volume.
 *
 * Packs the 7-bit volume (0–127) into the high byte of slot 0
 * (@c (arg0 & 0x7F) << 8) per the AKAO 16-bit-packed-param convention,
 * then dispatches.
 *
 * @param arg0  Target CD/XA volume (0–127).
 *
 * @see https://decomp.me/scratch/3oPkP (100%)
 */
s32 akao_cmd_e4_set_cd_volume(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)((arg0 & 0x7F) << 8);
    return akao_send_command(0xE4);
}

/**
 * @brief AKAO command 0xE5 — (a, 7-bit value packed into <<8).
 *
 * @see https://decomp.me/scratch/7PxF8 (100%)
 */
s32 akao_cmd_e5(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = (void*)(arg0);
    temp_a1 = (arg1 & 0x7F) << 8;
    g_akaoCmdParams[1] = (void*)(temp_a1);
    return akao_send_command(0xE5);
}

/**
 * @brief AKAO command 0xE6 — (8-bit value packed into <<8).
 *
 * @see https://decomp.me/scratch/XeUon (100%)
 */
s32 akao_cmd_e6(s32 arg0)
{
    g_akaoCmdParams[0] = (void*)((arg0 & 0xFF) << 8);
    return akao_send_command(0xE6);
}

/**
 * @brief Magic-checks an AKAO XA program and stages it for the SPU.
 *
 * After verifying the AKAO magic, picks a hardcoded SPU base
 * (@c 0x50900 if @p arg1 != 0, otherwise @c 0x43100) — and biases it by
 * @c 0xFFFD0000 when channel 0's @c flags & 0x40 is set with any in-flight
 * activity. Programs @c SpuSetTransferStartAddr, kicks off the sample upload
 * (akao_spu_write), caches the SPU base back into the buffer's
 * @c cached_spu_addr field, then memcpys the 0x50-byte header to the staging
 * area @c D_8004C150.
 *
 * @param arg0  Pointer to an AKAO buffer in main RAM.
 * @param arg1  Selects the upper SPU slot (non-zero) vs the lower slot.
 *
 * @return 0 on success; the akao_check_magic delta on failure (also clears
 *         @c D_8004C170).
 *
 * @see https://decomp.me/scratch/C06sg (99.80%)
 */
s32 akao_upload_xa_program(void* arg0, s32 arg1)
{
    s32 temp_v0;
    s32 var_s2;
    s32 var1;

    temp_v0 = akao_check_magic(arg0);
    if (temp_v0 == 0)
    {
        akao_spu_wait();
        var_s2 = 0x50900;
        if (arg1 == 0)
        {
            var_s2 = 0x43100;
        }
        /* Channel 0 has any in-flight activity AND its "active" flag bit is set. */
        if (((AKAO_CHANNEL_STATE->unk4 | AKAO_CHANNEL_STATE->unk1C) != 0) && (AKAO_CHANNEL_STATE->flags & 0x40))
        {
            var_s2 += 0xFFFD0000;
        }
        var1 = arg0;
        arg0 = (u8*)arg0 + 0x40;
        SpuSetTransferStartAddr(var_s2);
        /*
         * Note: this passes spu_dest_addr (offset 0x10) as the byte-count
         * argument to akao_spu_write, which is suspicious — the analogous
         * site in akao_upload_bank uses sample_size (offset 0x14) here. This
         * mirrors the original ASM literally; it may explain why this scratch
         * is at 99.80% rather than 100%. See docs/akao-review.md.
         */
        akao_spu_write(arg0, ((AkaoBankHeader*)var1)->spu_dest_addr);
        ((AkaoBankHeader*)var1)->cached_spu_addr = var_s2;
        func_80029A0C(var1, &D_8004C150, 0x50);
        return temp_v0;
    }
    D_8004C170 = 0;
    return temp_v0;
}

/**
 * @brief AKAO command 0xED — (8-bit value packed into <<8, b).
 *
 * @see https://decomp.me/scratch/ULEGL (100%)
 */
s32 akao_cmd_ed(s32 arg0, s32 arg1)
{
    g_akaoCmdParams[0] = (void*)((s32)((arg0 & 0xFF) << 8));
    g_akaoCmdParams[1] = (void*)(arg1);
    return akao_send_command(0xED);
}

/**
 * @brief AKAO command 0xEC — magic-checked AKAO buffer with mode flags.
 *
 * Picks a hardcoded SPU base (@c 0x50900 if @p upper_slot != 0, else
 * @c 0x43100), biases by @c 0xFFFD0000 when channel 0 is active and busy,
 * then dispatches with (buf, 8-bit packed into <<8, spu_base, arg3).
 *
 * @param buf        Pointer to an AKAO buffer in main RAM (validated via
 *                   akao_check_magic).
 * @param arg1       8-bit value packed into bits 8..15 of slot 1. TODO:
 *                   meaning unknown.
 * @param upper_slot Selects the upper SPU slot (non-zero) vs the lower slot.
 * @param arg3       Passed through verbatim into slot 3. TODO: meaning
 *                   unknown.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/SgcFo
 */
void akao_cmd_ec(void* buf, s32 arg1, s32 upper_slot, s32 arg3)
{
    s32 spu_base;

    if (akao_check_magic(buf) != 0)
    {
        return;
    }

    spu_base = upper_slot == 0 ? 0x43100 : 0x50900;

    if (((AKAO_CHANNEL_STATE->unk4 | AKAO_CHANNEL_STATE->unk1C) != 0) && (AKAO_CHANNEL_STATE->flags & 0x40))
    {
        spu_base += 0xFFFD0000;
    }

    g_akaoCmdParams[0] = (void*)(buf);
    g_akaoCmdParams[1] = (void*)((s32)((arg1 & 0xFF) << 8));
    g_akaoCmdParams[2] = (void*)(spu_base);
    g_akaoCmdParams[3] = (void*)(arg3);
    akao_send_command(0xEC);
}

/**
 * @brief AKAO command 0xE8 — begin XA-streamed AKAO playback.
 *
 * Validates @p arg1 != 0, disables SPU IRQ, primes the XA tracker
 * (@c g_akao_xa_tracker) for a stream of @c arg1 / 0x1000 frames, and dispatches.
 *
 * @param arg0  Stream identifier / control word in slot 0.
 * @param arg1  Total stream byte length (frame count = arg1 >> 12).
 *
 * @return 0 on success, -1 if @p arg1 is 0.
 *
 * @see https://decomp.me/scratch/bRIJX (100%)
 */
s32 akao_cmd_e8_start_xa_stream(s32 arg0, u32 arg1)
{
    if (arg1 == 0)
    {
        return -1;
    }
    SpuSetIRQ(0);
    SpuSetIRQAddr(0);
    g_akaoCmdParams[0] = (void*)(arg0);
    g_akaoCmdParams[1] = (void*)(arg1);
    g_akao_xa_tracker.unk34 = -1;
    g_akao_xa_tracker.unk20 = 0;
    g_akao_xa_tracker.unk24 = 0;
    g_akao_xa_tracker.unk28 = 0;
    g_akao_xa_tracker.unk38 = 0;
    g_akao_xa_tracker.unk3C = (s32)(arg1 >> 12);
    akao_send_command(0xE8);
    return 0;
}

/**
 * @brief Advances one frame of an in-flight XA-streamed AKAO sequence.
 *
 * Increments @c g_akao_xa_tracker.unk24 (frame count) and the per-frame index
 * @c .unk38, wrapping at @c .unk3C - 1; once two frames have streamed and
 * bit 0x01000000 of @c .unk8 is set, calls @c func_8002E2E8 to refill the
 * SPU ring buffer.
 *
 * @return @c D_8004F794 (the streaming-status latch read by callers).
 *
 * @see https://decomp.me/scratch/gKZ5G (100%)
 */
s32 akao_xa_advance_frame(void)
{
    u32 temp_v1;

    g_akao_xa_tracker.unk24 = g_akao_xa_tracker.unk24 + 1;
    temp_v1 = g_akao_xa_tracker.unk38 + 1;
    g_akao_xa_tracker.unk38 = temp_v1;
    if ((u32)(g_akao_xa_tracker.unk3C - 1) < temp_v1)
    {
        g_akao_xa_tracker.unk38 = 0;
    }
    if ((g_akao_xa_tracker.unk8 & 0x01000000) && ((u32)g_akao_xa_tracker.unk38 >= 2U))
    {
        func_8002E2E8(&g_akao_xa_tracker);
    }
    return D_8004F794;
}

/**
 * @brief Returns the current XA-stream position latch (@c D_8004F794).
 *
 * @see https://decomp.me/scratch/2DiS3 (100%)
 */
s32 akao_xa_get_position(void)
{
    return D_8004F794;
}
