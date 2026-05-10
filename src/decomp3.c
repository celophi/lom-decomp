#include "decomp3.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/hDNyF
 */
s32 FUN_80021fbc(void)
{
    func_80023AD0();
    return 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/z7ZEh
 */
s32 func_80021FDC(void)
{
    func_80023BE0();
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

    temp_v0 = akao_check_magic();
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
    g_akaoCmdParams[0] = seqData;
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
    g_akaoCmdParams[0] = arg0;
    akao_send_command(0x11);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/4GVez
 */
void func_80022090(void)
{
    akao_send_command(0x40);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/c2C3m
 */
void func_800220B0(s32 arg0, s32 arg1)
{
    g_akaoCmdParams[0] = arg0;
    g_akaoCmdParams[1] = arg1;
    g_akaoCmdParams[2] = 0;
    akao_send_command(0x14);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/d6xXt
 */
s32 func_800220E4(s32 arg0, s32 arg1)
{
    s32 temp_v0;

    g_akaoCmdParams[0] = arg0;
    temp_v0 = akao_send_command(0x19);
    g_akaoCmdParams[0] = (s32)(arg1 & 0x7F);
    g_akaoCmdParams[3] = 0;
    akao_send_command(0xC0);
    return temp_v0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/jigab
 */
void func_8002213C(s32 arg0, s32 arg1)
{
    g_akaoCmdParams[0] = arg0;
    g_akaoCmdParams[1] = arg1;
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

    g_akaoCmdParams[0] = (s32)(arg0 & 0x3FF);
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0xFF;
    temp_a3 = arg3 & 0x7F;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = temp_a2;
    g_akaoCmdParams[3] = temp_a3;
    akao_send_command(0x20);
};

/**
 * decomp.me link (100%) https://decomp.me/scratch/FFGei
 */
s32 func_800221BC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 result = akao_check_magic(arg0);

    if (result != 0)
    {
        return result;
    }

    g_akaoCmdParams[0] = arg0;
    g_akaoCmdParams[1] = arg1 & 0xFFFFFF;
    g_akaoCmdParams[2] = arg2 & 0xFF;
    g_akaoCmdParams[3] = arg3 & 0x7F;
    akao_send_command(0x24);

    return arg0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/lu9nS
 */
void func_80022240(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    g_akaoCmdParams[1] = temp_a1;
    akao_send_command(0x21);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/0mLzI
 */
void func_8002227C(s32 arg0)
{
    g_akaoCmdParams[0] = arg0 & 0x3FF;
    akao_send_command(0x30);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/yZloM
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
    ptr = D_8004B430;
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
 * decomp.me link (100%) https://decomp.me/scratch/OvqYq
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
    ptr = D_8004B430;
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
 * decomp.me link (100%) https://decomp.me/scratch/9qTjH
 */
void func_8002237C(s32 arg0)
{
    int* new_var2;
    int new_var;
    new_var = (arg0 == 1) ? (0x81) : (0x80);
    new_var2 = &new_var;
    akao_send_command(new_var);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/x94md
 */
void func_800223B0(s32 arg0)
{
    g_akaoCmdParams[0] = arg0;
    akao_send_command(0x90);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/y9TAf
 */
void func_800223D8(s32 arg0)
{
    g_akaoCmdParams[0] = arg0;
    akao_send_command(0x92);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/qqSuG
 */
void FUN_80022400(u32 param_1)
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
 * decomp.me link (100%) https://decomp.me/scratch/iREFc
 */
void func_8002246C(u32 arg0)
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
 * decomp.me link (100%) https://decomp.me/scratch/VTGCB
 */
void func_800224D8(s32 arg0)
{
    g_akaoCmdParams[0] = arg0 & 0x7F;
    akao_send_command(0xA8);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/03hNO
 */
void func_80022504(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0x7F;
    g_akaoCmdParams[1] = temp_a1;
    akao_send_command(0xA9);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/C8UTP
 */
void func_80022538(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0x7F;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = temp_a2;
    akao_send_command(0xA0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/xMNn0
 */
void func_8002257C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a3;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a3 = arg3 & 0x7F;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = arg2;
    g_akaoCmdParams[3] = temp_a3;
    akao_send_command(0xA1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/AuyLX
 */
void func_800225C4(s32 arg0)
{
    g_akaoCmdParams[0] = arg0 & 0xFF;
    akao_send_command(0xAA);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/IaBX9
 */
void func_800225F0(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    akao_send_command(0xAB);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/LhoLV
 */
void func_80022624(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = temp_a2;
    akao_send_command(0xA2);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Al5YT
 */
void func_80022668(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a3;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a3 = arg3 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = arg2;
    g_akaoCmdParams[3] = temp_a3;
    akao_send_command(0xA3);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/e4D90
 */
void func_800226B0(s32 arg0)
{
    g_akaoCmdParams[0] = arg0 & 0xFF;
    akao_send_command(0xAC);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/Fw2d9
 */
void func_800226DC(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    akao_send_command(0xAD);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/vHMVZ
 */
s32 func_80022710(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = temp_a2;
    return akao_send_command(0xA4);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/exTVG
 */
s32 func_80022754(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_a3;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFFFFFF;
    temp_a3 = arg3 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = arg2;
    g_akaoCmdParams[3] = temp_a3;
    return akao_send_command(0xA5);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/QPqUd
 */
s32 FUN_8002279c(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0x7F;
    g_akaoCmdParams[1] = temp_a1;
    return akao_send_command(0xC0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/cSIwP
 */
s32 func_800227D0(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a2;

    g_akaoCmdParams[0] = arg0;
    temp_a2 = arg2 & 0x7F;
    g_akaoCmdParams[1] = arg1;
    g_akaoCmdParams[2] = temp_a2;
    return akao_send_command(0xC1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/PbMJC
 */
s32 func_80022808(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a2;
    s32 temp_a3;

    g_akaoCmdParams[0] = arg0;
    temp_a2 = arg2 & 0x7F;
    temp_a3 = arg3 & 0x7F;
    g_akaoCmdParams[1] = arg1;
    g_akaoCmdParams[2] = temp_a2;
    g_akaoCmdParams[3] = temp_a3;
    return akao_send_command(0xC2);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/BeJR1
 */
s32 func_80022848(s32 arg0)
{
    g_akaoCmdParams[0] = arg0;
    return akao_send_command(0xC8);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/yo40G
 */
s32 func_80022870(s32 arg0, s32 arg1)
{
    g_akaoCmdParams[0] = arg0;
    g_akaoCmdParams[1] = arg1;
    return akao_send_command(0xC9);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/pLMBi
 */
s32 func_800228A0(s32 arg0, s32 arg1, s32 arg2)
{
    g_akaoCmdParams[0] = arg0;
    g_akaoCmdParams[1] = arg1;
    g_akaoCmdParams[2] = arg2;
    return akao_send_command(0xCA);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/klUxi
 */
s32 func_800228D4(s32 arg0)
{
    g_akaoCmdParams[0] = arg0 & 0xFF;
    return akao_send_command(0xD0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/XXHwt
 */
s32 func_80022900(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    return akao_send_command(0xD1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/074UT
 */
s32 func_80022934(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = temp_a2;
    return akao_send_command(0xD2);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/yJdLv
 */
s32 func_80022970(s32 arg0)
{
    g_akaoCmdParams[0] = arg0 & 0xFF;
    return akao_send_command(0xD4);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/u6Eys
 */
s32 func_8002299C(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    return akao_send_command(0xD5);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/ITNFU
 */
void func_800229D0(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = temp_a2;
    akao_send_command(0xD6);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/JS2nD
 */
s32 func_80022A0C(s32 arg0)
{
    g_akaoCmdParams[0] = arg0 & 0xFF;
    return akao_send_command(0xD8);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/YD6rZ
 */
s32 func_80022A38(s32 arg0, s32 arg1)
{
    s32 temp_a1;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    return akao_send_command(0xD9);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/jzW0l
 */
s32 func_80022A6C(s32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_a1;
    s32 temp_a2;

    g_akaoCmdParams[0] = arg0;
    temp_a1 = arg1 & 0xFF;
    temp_a2 = arg2 & 0xFF;
    g_akaoCmdParams[1] = temp_a1;
    g_akaoCmdParams[2] = temp_a2;
    return akao_send_command(0xDA);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/dgbnE
 */
s32 FUN_80022aa8(void)
{
    return akao_send_command(0xF0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/IMYAL
 */
s32 FUN_80022ac8(void)
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
    D_8004F750 &= ~1;
    while (akao_submit(sequenceData, waitForCompletion) == 1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/ecQHb
 */
s32 func_80022B48(void)
{
    return D_8003EC4C;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/qBE70
 */
s32 func_80022B58(void)
{
    D_8004F824 = 0;
    D_8004F750 |= 1;
    return 0;
}

/**
 * decomp.me link (98.59%) https://decomp.me/scratch/BEUjs
 */
s32 func_80022B78(s32 arg0, u32 arg1, s32 arg2)
{
    s32 temp_v0;
    u32 var_s0;
    u32 var_v1;
    s32* new_var;
    void* temp_a0_2;
    if ((D_8004F750 & 1) == 0)
    {
        return D_8004F828;
    }
    if (D_8004F820.unk4 == 0)
    {
        if (akao_check_magic(arg0) == 0)
        {
            func_80029A0C(*(new_var = &arg0), &D_8004D3C0, 0x40U);
            arg0 += 0x40;
            arg1 -= 0x40;
            D_8004F820.unk4 = (s32)D_8004D3C0.unk10;
            D_8004F820.unk8 = (u32)D_8004D3C0.unk14;
            D_8004F820.unk0 = (void*)((D_8004D3C0.unk18 * 0x10) + ((u32)(&D_8004C340)));
            D_8004F820.unkC = (u32)(D_8004D3C0.unk1C * 0x10);
        }
        else
        {
            arg1 = 0;
            D_8004F820.unk8 = 0U;
            D_8004F820.unkC = 0U;
        }
    }
    if (D_8004F820.unkC != 0)
    {
        var_s0 = D_8004F820.unkC;
        if (arg1 != 0)
        {
            if (var_s0 >= arg1)
            {
                var_s0 = arg1;
            }
            func_80029A0C(arg0, D_8004F820.unk0, var_s0);
            temp_v0 = (var_s0 >> 2) * 4;
            arg0 += temp_v0;
            arg1 -= var_s0;
            D_8004F820.unk0 = (void*)(((u32)D_8004F820.unk0) + temp_v0);
            D_8004F820.unkC -= var_s0;
            if (D_8004F820.unkC == 0)
            {
                temp_a0_2 = (void*)((D_8004D3C0.unk18 * 0x10) + ((u32)(&D_8004C340)));
                func_800235A8(temp_a0_2, temp_a0_2, D_8004D3C0.unk10, D_8004D3C0.unk1C);
            }
        }
    }
    if (arg1 != 0)
    {
        if (D_8004F820.unk8 != 0)
        {
            var_v1 = D_8004F820.unk8;
            if (D_8004F820.unk8 >= arg1)
            {
                var_v1 = arg1;
            }
            var_s0 = var_v1;
            SpuSetTransferStartAddr(D_8004F820.unk4);
            func_80023660(arg0, arg1);
            D_8004F820.unk4 += var_v1;
            D_8004F820.unk8 -= var_s0;
            if (arg2 != 0)
            {
                func_800236EC();
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
        D_8004F750 &= ~1;
    }
    return D_8004F828;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/0f3IK
 */
s32 func_80022D8C(AkaoSeqHeader* sequenceData, s32 waitForCompletion)
{
    akao_play_sequence_blocking(sequenceData, waitForCompletion);
    return 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/FWcdy
 */
s32 func_80022DAC(void* arg0, s32 arg1, s32 arg2)
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

    func_8002376C(arg0, arg2, var_a2, var_a3_2);
    return 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/sa1fh
 */
s32 func_80022ED8(void* arg0, s32 arg1, s32 arg2)
{
    func_80022DAC(arg0, arg1, arg2);
    return 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/PnDWc
 */
s32 func_80022EF8(void* arg0, s32 arg1, s32 arg2)
{
    func_80022DAC(arg0, arg1 + 3, arg2);
    return 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/hcfmi
 */
s32 func_80022F18(s32 arg0, void* arg1)
{
    s32 new_var;
    u32 temp_v0;
    s32 val = arg0;
    new_var = val;
    if (D_8004F754 & 2)
    {

        D_8003EC20.val3 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
        D_8003EC20.val1 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
        D_8003EC20.val2 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
        D_8003EC20.val0 = (u_char)((unsigned long long)(((u32)(arg0 * 0xB570)) >> 0x11));
    }
    else
    {
        D_8003EC20.val2 = (u_char)new_var;
        D_8003EC20.val0 = (u_char)new_var;
        D_8003EC20.val3 = 0;
        D_8003EC20.val1 = 0;
    }
    CdMix(&D_8003EC20);
    return 0;
}