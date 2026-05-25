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
 * 68% match with GNU AS
 * Note that this could be probably FUNCTIONALLY INCORRECT!
 */
s32 func_80050B14(s32 arg0)
{
    s32 new_var2;
    s32 state = 0;

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

    case 3: /* Wait for response on screen 6; on confirm, set command byte from display mode and show screen 2 */
        state = PollCdResponse(6);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;
        case 1:
            g_CmdBuf[0] = (g_displayMode >= 2) ? 2 : 0;
            SendCdCommand(2);
            g_checkPSState = 4;
        /* fall through */
        case 0:
            state = 3;
            break;
        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;
        default:
            state = 3;
            break;
        }
        break;

    case 4: /* Wait for response on screen 2; on confirm, decode RTC BCD time into g_timeBuffer (halved) and show screen
               0xC */
        state = PollCdResponse(2);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        case 1:
        {
            volatile u8 *bcd;
            u8 bcd0;
            u8 bcd1;
            s32 h;
            s32 m;
            s32 t;
            s32 hb;
            s32 mb;

            bcd = (volatile u8 *)g_RTCTimeBCD;
            bcd0 = bcd[0];
            bcd1 = bcd[1];

            h = ((bcd0 >> 4) * 10) + (bcd0 & 0xF);
            m = (((bcd1 >> 4) * 5) * 2) + (bcd1 & 0xF);
            t = ((h * 60) + m) >> 1;
            hb = t / 60;
            mb = t % 60;

            g_timeBuffer[0] = hb;
            g_timeBuffer[0] = ((g_timeBuffer[0] / 10) << 4) | (g_timeBuffer[0] % 10);

            g_timeBuffer[1] = mb;
            g_timeBuffer[1] = ((g_timeBuffer[1] / 10) << 4) | (g_timeBuffer[0] % 10);

            SendCdCommand(0xC);
            state = 7;
            g_checkPSState = 5;
            break;
        }

        case 0:
        default:
            state = 7;
            break;
        }

        break;

    case 5: /* Wait for screen 0xC; on confirm button (statusFlag bit 6), fill g_CmdBuf with encoded time and send */
    {
        D_8005CFE0_t* base;
        state = PollCdResponse(0xC);
        base = &g_statusFlag;
        switch (state)
        {
        case -1:
            if (!(base->unk0 & 1))
            {
                g_checkPSState = 1;
                state = -1;
            }
            else if (base->unk1 & 0x40)
            {
                volatile u8 *cmd = (volatile u8 *)g_CmdBuf;
                state = 5;
                cmd[2] = 0;
                cmd[0] = g_timeBuffer[0];
                cmd[1] = g_timeBuffer[1];
                SendCdCommand(3);
                g_checkPSState = 7;
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
            ExitCheckPS();
            state = -1;
            break;

        case 0:
            do
            {
                state = 6;
                break;
            } while (0);
            break;

        case 1:
        {
            g_CmdBuf[2] = 0;
            g_CmdBuf[0] = g_timeBuffer[0];
            g_CmdBuf[1] = g_timeBuffer[1];
            SendCdCommand(3);
            g_checkPSState = 7;
            state = 6;
            break;
        }

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

    case 7: /* Wait for screen 3 response; on confirm, set g_CmdBuf[0]=1 and show screen 5 */
        state = PollCdResponse(3);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            g_CmdBuf[0] = (u8)state;
            SendCdCommand(5);
            g_checkPSState = 8;
        /* fall through */
        case 0:
            state = 7;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 7;
            break;
        }

        break;

    case 8: /* Wait for screen 5 response; on confirm, record vsync timestamp and advance */
        state = PollCdResponse(5);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            g_vsyncTimestamp = VSync(-1);
            g_checkPSState = 9;
        /* fall through */
        case 0:
            state = 8;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 8;
            break;
        }

        break;

    case 9: /* Wait 3 vsyncs, then show screen 4 */

        new_var2 = (g_vsyncTimestamp + 3) < VSync(-1);
        if (new_var2)
        {
            SendCdCommand(4);
            g_checkPSState = 0xA;
        }
        state = 9;
        break;

    case 10: /* Wait for screen 4 response; on confirm, show screen 7 */
        state = PollCdResponse(4);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            SendCdCommand(7);
            g_checkPSState = 0xB;
        /* fall through */
        case 0:
            state = 0xA;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xA;
            break;
        }

        break;

    case 11: /* Wait for screen 7 response; on confirm, show screen 8 */
        state = PollCdResponse(7);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            SendCdCommand(8);
            g_checkPSState = 0xC;
        /* fall through */
        case 0:
            state = 0xB;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xB;
            break;
        }

        break;

    case 12: /* Wait for screen 8 response; on confirm, set g_CmdBuf[0]=4 and show screen 9 */
        state = PollCdResponse(8);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            g_CmdBuf[0] = 4;
            SendCdCommand(9);
            g_checkPSState = 0xD;
        /* fall through */
        case 0:
            state = 0xC;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xC;
            break;
        }

        break;

    case 13: /* Wait for screen 9 response; on confirm, record vsync timestamp and advance */
        state = PollCdResponse(9);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            g_vsyncTimestamp = VSync(-1);
            g_checkPSState = 0xE;
        /* fall through */
        case 0:
            state = 0xD;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xD;
            break;
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

    case 15: /* Wait for screen 0xA response; on confirm, branch on whether RTC hours is nonzero */
        state = PollCdResponse(10);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
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
        /* fall through */
        case 0:
            state = 15;
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 17;
            state = -1;
            break;

        default:
            state = 15;
            break;
        }

        break;

    case 16: /* Wait for screen 0 response */
        state = PollCdResponse(0);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 0:
            state = 16;
            break;

        case 1:
            state = 16;
            ExitCheckPS();
            break;

        case -2:
            SendCdCommand(0);
            g_checkPSState = 17;
            state = -1;
            break;

        default:
            state = 16;
            break;
        }

        break;

    case 17: /* Wait for screen 0 response (retry path); on confirm, restart state machine from state 1 */
        state = PollCdResponse(0);
        switch (state)
        {
        case 1:
            g_checkPSState = (u32)state;
            state = 16;
            break;

        case -1:
            g_checkPSState = 1;
            state = -1;
            break;
        case -2:
            SendCdCommand(0);
            state = 16;
            break;

        case 0:
            state = 16;
            break;

        default:
            state = 16;
            break;
        }

        break;

    case 18: /* Wait for screen 0xB response; on confirm, reset state machine to state 0 */
        state = PollCdResponse(0xB);
        switch (state)
        {
        case -2:
            SendCdCommand(0);
            g_checkPSState = 0x11;
            state = -1;
            break;
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 0:
            state = 0x12;
            break;

        case 1:
            g_checkPSState = 0;

        default:
            state = 0x12;
            break;
        }

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
                        __asm__ __volatile__ ("" : : : "$2");

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

    for (i = 0; i < 4; i++)
        *ptr = i;

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