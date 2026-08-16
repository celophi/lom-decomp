#include "checkps.h"

/**
 * Pointer to CD-ROM status register and related I/O ports for communicating with the PS1's CD drive.
 * CD Index/Status Register (Bit0-1 R/W, Bit2-7 Read Only)
 */
s8* g_cdStatusRegister = (s8*)0x1F801800;

/**
 * Pointer to CD-ROM response register, used for reading data returned by the CD drive after issuing commands.
 * CD Response Fifo (R) (usually with Index1)
 */
u8* g_cdResponseRegister = (s8*)0x1F801801;

/**
 * Pointer to CD-ROM data register, used for sending command parameters to the CD drive after writing a command to the status register.
 * CD Data Fifo - 8bit/16bit (R) (usually with Index0..1)
 */
u8* g_cdDataRegister = (u8*)0x1F801802;

/**
 * Pointer to CD-ROM IRQ register, used for handling CD drive interrupts.
 * CD IRQ Register (R/W)
 */
s8* g_cdIrqRegister = (s8*)0x1F801803;

s32 g_vsyncTimestamp = 0;

s32 g_displayMode = 0;

u8 g_timeBuffer[2] = {0};

/**
 * @brief CheckPS state machine: drives the RTC clock-set screens via CD commands.
 *
 * 98.11% match with GCC 2.7.2 + GNU AS (gcc272_gnu). See
 * working/func_80050B14/STATUS.md for the remaining mismatch analysis.
 *
 * @param arg0 Nonzero to run a single step; zero to loop until state reaches 0.
 * @return Current state value (-1, or the active state number).
 *
 * @note The backward goto is required to match: a syntactic do/while triggers
 *       gcc's loop optimizer (invariant hoisting into s-regs, frame -0x30)
 *       and drops the match to 81%.
 */
s32 func_80050B14(s32 arg0)
{
    s32 new_var2;
    s32 state = 0;
    static void* keep_fall3 = &&fall3;
    static void* keep_fall4 = &&fall4;
    static void* keep_fall7 = &&fall7;
    static void* keep_fall8 = &&fall8;
    static void* keep_fall10 = &&fall10;
    static void* keep_fall11 = &&fall11;
    static void* keep_fall12 = &&fall12;
    static void* keep_fall13 = &&fall13;
    static void* keep_fall15 = &&fall15;
    static void* keep_fall16 = &&fall16;
    static void* keep_fall17 = &&fall17;
    static void* keep_fall18 = &&fall18;
    static void* keep_cmd6_body = &&cmd6_body;
    static void* keep_s15_body = &&s15_body;

loop:
    switch (g_checkPSState)
    {
    case 1: /* Init — show opening screen, advance to state 2 */
        SendCdCommand(1);
        g_checkPSState = 2;
        state = 1;
        break;

    case 2: /* Wait for response on screen 1; on confirm, latch clock mode and show screen 6 */
        state = PollCdResponse(1);
        switch (state)
        {
        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = 2;
            break;

        case 0:
            state = 2;
            break;

        case 1:
            g_displayMode = g_clockMode;
            state = 2;
            SendCdCommand(6);
            g_checkPSState = 3;
            break;

        case -1:
            SendCdCommand(1);
            state = 2;
            break;

        default:
            state = 2;
            break;
        }

        break;

    case 3:
        state = PollCdResponse(6);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall3:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 3;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        g_CmdBuf[0] = (g_displayMode >= 2) ? 2 : 0;
                        SendCdCommand(2);
                        g_checkPSState = 4;
                    }
                }
                state = 3;
            }
        }
        break;

    case 4:
        state = PollCdResponse(2);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall4:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 7;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        {

                            u8* bcd;
                            u8 bcd0;
                            u8 bcd1;
                            u32 x;
                            u32 y;
                            u32 z;
                            s32 h;
                            s32 m;
                            s32 t;
                            s32 hb;
                            s32 mb;
                            int idx0;
                            int idx1;
                            int idx2;
                            int idx3;
                            int idx4;
                            int idx5;
                            u32 seed;

                            seed = ((u32)0x88888889U + (u32)arg0) - (u32)arg0;
                            bcd = (u8*)((u32)g_RTCTimeBCD + (seed ^ (u32)0x88888889U));
                            bcd0 = bcd[0];
                            bcd1 = bcd[1];

                            h = ((bcd0 >> 4) * 10) + (bcd0 & 0xF);
                            m = (((bcd1 >> 4) * 5) * 2) + (bcd1 & 0xF);
                            t = ((h * 60) + m) >> 1;
                            hb = t / 60;
                            mb = t % 60;

                            /* Volatile-CAST stores + plain reads (tb_A shape): stores stay
                               un-forwarded and reads need no andi split. The array itself
                               must be non-volatile for the reads to come out clean. */
                            idx0 = 0;
                            *(volatile u8*)&g_timeBuffer[idx0] = hb;
                            idx1 = 0;
                            x = g_timeBuffer[idx1];
                            idx4 = 1;
                            *(volatile u8*)&g_timeBuffer[idx4] = mb;
                            idx5 = 1;
                            y = g_timeBuffer[idx5];
                            idx2 = 0;
                            *(volatile u8*)&g_timeBuffer[idx2] = ((x / 10) << 4) | (x % 10);
                            idx3 = 0;
                            z = g_timeBuffer[idx3];
                            g_timeBuffer[1] = ((y / 10) << 4) | (z % 10);

                            SendCdCommand(0xC);
                            g_checkPSState = 5;
                        }
                    }
                }
                state = 7;
            }
        }
        break;

    case 5: /* Wait for screen 0xC; on confirm button (statusFlag bit 6), fill g_CmdBuf with encoded time and send */
    {
        state = PollCdResponse(0xC);
        switch (state)
        {
        case -1:
        {
            /* Post-increment keeps sf twice-set and every access a bare
               (mem (reg)), so both flag bytes read through one base
               register as in the original (required to match). */
            u8* sf = (u8*)&g_statusFlag;
            u8* base_sf = sf;
            if (base_sf[0] & 1)
            {
                if (*++sf & 0x40)
                {
                    u8 tb0 = g_timeBuffer[0];
                    u8 tb1 = g_timeBuffer[1];
                    u8* cmd;
                    u32 q;
                    state = 5;
                    q = ((u32)g_CmdBuf + (u32)arg0) - (u32)arg0;
                    cmd = (u8*)q;
                    cmd[2] = 0;
                    *cmd++ = tb0;
                    *cmd = tb1;
                    SendCdCommand(3);
                    g_checkPSState = 7;
                }
            }
            else
            {
                new_var2 = 1;
                g_checkPSState = new_var2;
                state = -1;
            }
        }
        break;

        case 0:
            state = 5;
            break;

        case 1:
            SendCdCommand(0xD);
            g_checkPSState = 6;
            state = 5;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 5;
            break;
        }

        break;
    }

    case 6: /* Wait for screen 0xD; on confirm, fill g_CmdBuf with encoded time and send */
        state = PollCdResponse(0xD);
        switch (state)
        {
        case -1:
            ((s32 (*)(void))ExitCheckPS)();
            state = -1;
            break;

        case 1:
        {
            u8 tb0;
            u8 tb1;
            u8* cmd;
            u32 q;
            state = 6;
        cmd6_body:
            tb0 = g_timeBuffer[0];
            tb1 = g_timeBuffer[1];
            q = ((u32)g_CmdBuf + (u32)arg0) - (u32)arg0;
            cmd = (u8*)q;
            cmd[2] = 0;
            *cmd++ = tb0;
            *cmd = tb1;
            SendCdCommand(3 + ((state ^ arg0) ^ state ^ arg0));
            g_checkPSState = 7;
        }
        /* fall through */
        case 0:
            state = 6;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 6;
            break;
        }

        break;

    case 7:
        state = PollCdResponse(3);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall7:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 7;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        g_CmdBuf[0] = (u8)state;
                        SendCdCommand(5);
                        g_checkPSState = 8;
                    }
                }
                state = 7;
            }
        }
        break;

    case 8:
        state = PollCdResponse(5);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall8:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 8;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        g_vsyncTimestamp = VSync(-1);
                        g_checkPSState = 9;
                    }
                }
                state = 8;
            }
        }
        break;

    case 9: /* Wait 3 vsyncs, then show screen 4 */

        new_var2 = VSync(-1);
        if ((g_vsyncTimestamp + 3) < new_var2)
        {
            SendCdCommand(4);
            g_checkPSState = 0xA;
        }
        state = 9;
        break;

    case 10:
        state = PollCdResponse(4);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall10:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 0xA;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        SendCdCommand(7);
                        g_checkPSState = 0xB;
                    }
                }
                state = 0xA;
            }
        }
        break;

    case 11:
        state = PollCdResponse(7);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall11:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 0xB;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        SendCdCommand(8);
                        g_checkPSState = 0xC;
                    }
                }
                state = 0xB;
            }
        }
        break;

    case 12:
        state = PollCdResponse(8);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall12:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 0xC;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        g_CmdBuf[0] = 4;
                        SendCdCommand(9);
                        g_checkPSState = 0xD;
                    }
                }
                state = 0xC;
            }
        }
        break;

    case 13:
        state = PollCdResponse(9);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall13:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 0x11;
                    state = -1;
                }
                else
                {
                    state = 0xD;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        g_vsyncTimestamp = VSync(-1);
                        g_checkPSState = 0xE;
                    }
                }
                state = 0xD;
            }
        }
        break;

    case 14: /* Wait ~200 vsyncs (~3.3s), then set g_CmdBuf[0]=5 and show screen 0xA */
        new_var2 = VSync(-1);
        if ((g_vsyncTimestamp + 0xC8) < new_var2)
        {
            g_CmdBuf[0] = 5;
            SendCdCommand(0xA);
            g_checkPSState = 0xF;
        }
        state = 0xE;
        break;

    case 15:
        state = PollCdResponse(10);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall15:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 17;
                    state = -1;
                }
                else
                {
                    state = 15;
                }
            }
            else
            {
                if (state != 0)
                {
                    if (state == 1)
                    {
                        state = 15;
                    s15_body:
                        if (g_RTCTimeBCD[0] != 0)
                        {
                            SendCdCommand(0);
                            g_checkPSState = 16;
                        }
                        else
                        {
                            g_vsyncTimestamp = VSync(-1);
                            g_checkPSState = 19;
                        }
                    }
                }
                state = 15;
            }
        }
        break;

    case 16:
        state = PollCdResponse(0);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall16:
            if (state < 0)
            {
                if (state == -2)
                {
                    SendCdCommand(0);
                    g_checkPSState = 17;
                    state = -1;
                }
                else
                {
                    state = 16;
                }
            }
            else
            {
                if (state == 0)
                    goto pos_default17;
                if (state != 1)
                {
                    state = 16;
                    break;
                }
                ExitCheckPS();
                state = 16;
                break;
            }
        }
        break;
        state = 16;
        break;

    case 17:
        state = PollCdResponse(0);
        if (state == -1)
        {
            new_var2 = 1;
            g_checkPSState = new_var2;
            state = -1;
        }
        else
        {
        fall17:
            if (state < 0)
            {
                if (state == -2)
                    goto neg17;
                state = 16;
                break;
            }
            else
            {
                if (state == 0)
                    goto pos_default17;
                if (state != 1)
                    goto pos_default17;
                g_checkPSState = (u32)state;
            pos_default17:
                state = 16;
                goto pos_exit17;
            }
        }
        break;
    neg17:
        SendCdCommand(0);
        state = 16;
        break;

    case 18: /* Wait for screen 0xB response; on confirm, reset state machine to state 0 */
        state = PollCdResponse(0xB);
        if (state == -1)
            goto neg18;
    fall18:
        if (state < 0)
        {
            if (state == -2)
                goto neg2_18;
            goto default18;
        }
        if (state == 0)
            goto default18;
        if (state == 1)
            goto one18;
        goto default18;
    neg2_18:
        SendCdCommand(0);
        g_checkPSState = 0x11;
        state = -1;
        break;
    neg18:
        new_var2 = 1;
        g_checkPSState = new_var2;
        state = -1;
        break;
    one18:
        g_checkPSState = 0;
    default18:
        state = 18;
        break;

    case 19: /* Wait 10 vsyncs, then show screen 0xB */
        new_var2 = VSync(-1);
        if ((g_vsyncTimestamp + 10) < new_var2)
        {
            SendCdCommand(11);
            g_checkPSState = 18;
        }
        state = 19;
        break;

    case 0: /* Idle / reset */
        state = 0;
        break;
    }

    if ((arg0 == 0) && (state != 0))
    {
        goto loop;
    }
    return state;
pos_exit17:
    if ((arg0 == 0) && (state != 0))
    {
        goto loop;
    }
    return state;
}

/**
 * GNU AS (99.91%) match
 */
s32 PollCdResponse(s32 arg0)
{
    u8 t2;
    u8 val1;
    u8 val2;
    s32 temp_v1;
    s32 temp_a2;
    s32 i;
    int new_var;
    s32 idx;
    s32 j;
    t2 = (&g_cdCmdTable->irqThresh)[arg0 * 4];
    *g_cdStatusRegister = 1;
    val1 = *((volatile u8*)g_cdIrqRegister);
    val2 = *((volatile u8*)g_cdIrqRegister);
    if ((new_var = val1 & 7) == (val2 & 7))
    {
        temp_v1 = new_var;
        temp_a2 = (unsigned char)temp_v1;
        if (temp_a2 != 0)
        {
            g_cdIrqAccum = g_cdIrqAccum + temp_a2;
            *g_cdStatusRegister = 1;
            *g_cdIrqRegister = 7;
            i = 0;
            do
            {
                *((int*)0) = i;
                i++;
            } while (i < 4);
            if (g_cdIrqAccum >= ((s32)t2))
            {
                j += 0;
                g_cdIrqAccum = 0;
                if (temp_v1 == 5)
                {
                    do
                    {
                        g_statusFlag.unk0 = *g_cdResponseRegister;
                    } while (0);
                    g_statusFlag.unk1 = *g_cdResponseRegister;
                    *g_cdStatusRegister = 1;
                    *g_cdDataRegister = 0x1F;
                    if (!(g_statusFlag.unk0 & 0x10))
                    {
                        return -1;
                    }

                    return -2;
                }
                else
                {
                    temp_a2 = 0;
                    idx = arg0 * 4;
                    j = temp_a2;
                    if ((&g_cdCmdTable->respCount)[idx] != temp_a2)
                    {
                        do
                        {
                            ((u8*)(&g_statusFlag))[j] = *g_cdResponseRegister;
                            j++;
                        } while (j < ((s32)(*((volatile u8*)(&(&g_cdCmdTable->respCount)[idx])))));
                    }
                    *g_cdStatusRegister = 1;
                    *g_cdDataRegister = 0x1F;
                    if (arg0 != 0xA)
                    {
                        j = g_statusFlag.unk0;

                        // HACK
                        __asm__ __volatile__("" : : : "$2");

                        j &= 0x10;
                        if (j)
                        {
                            idx = 2;
                            return -idx;
                        }
                    }
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}

/**
 * decomp.me link (83.42%%) https://decomp.me/scratch/vhWhP
 * Matches 100% with GNU AS
 */
void SendCdCommand(int arg0)
{
    s32 i = 0;
    s32* ptr = 0;
    s32 j;
    unsigned int idx;

    *g_cdStatusRegister = 1;
    *g_cdIrqRegister = 7;

    for (i = 0; i < 4; i++) *ptr = i;

    *g_cdStatusRegister = 1;
    *g_cdDataRegister = 0x18;
    *g_cdStatusRegister = 0;

    idx = arg0 * 4;

    j = 0;
    if ((&g_cdCmdTable->paramCount)[idx])
    {
        do
        {
            *g_cdDataRegister = g_CmdBuf[j];
            j++;
        } while (j < (&g_cdCmdTable->paramCount)[idx]);
    }

    *g_cdStatusRegister = 0;
    *g_cdResponseRegister = (&g_cdCmdTable->opcode)[arg0 * 4];
}

/**
 * decomp.me link (96.94%) https://decomp.me/scratch/rZ9Jk
 * Matches 100% with GNU AS
 */
void ExitCheckPS(void)
{
    DRAWENV sp18;
    DISPENV sp78;
    DR_ENV sp90;
    unsigned long new_var;
    u32 spD0[3];
    FourShorts spE;
    s32 var_a2;
    s32 var_s0;
    ResetGraph(1);
    StopCallback();
    ResetGraph(5);
    *((s16*)0x1F801DAA) = 0;
    SetDefDrawEnv(&sp18, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(&sp78, 0, 0, 0x140, 0xF0);
    sp18.isbg = 1;
    SetDrawEnv(&sp90, &sp18);
    DrawPrim(&sp90);
    PutDispEnv(&sp78);
    spD0[0] = 0x02000000;
    spD0[1] = 0xE6000002;
    spD0[2] = 0;
    DrawPrim(spD0);
    var_a2 = 0xFFFF;

    spE.c = 0x10;
    spE.d = 1;

    for (var_s0 = 0; var_s0 < 2; var_s0++)
    {
        spE.a = var_s0 + 0x50;
        spE.b = var_s0 + 0x5C;
        DrawString((const char*)&D_8004FCC4, &spE, var_a2);
        var_a2 = 0x8000;
    }

    DrawSymmetricTestPattern();
    SetDispMask(1);
    exit();
}