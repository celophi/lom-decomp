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
 * decomp.me link (96.97%) https://decomp.me/scratch/4tVNg
 */
s32 func_800221BC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    long long new_var2;
    s32 var_v0;
    unsigned char new_var;
    var_v0 = akao_check_magic();
    if (var_v0 == 0)
    {
        g_akaoCmdParams[0] = arg0;
        new_var = arg2;
        g_akaoCmdParams[1] = (s32)(arg1 & 0xFFFFFF);
        g_akaoCmdParams[2] = (s32)(new_var & 0xFF);
        g_akaoCmdParams[3] = (s32)(arg3 & 0x7F);
        akao_send_command(0x24);
        new_var2 = arg0;
        var_v0 = new_var2;
    }
    return var_v0;
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
