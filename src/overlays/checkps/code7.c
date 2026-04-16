#include "checkps.h"

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
    s32 h;
    s32 m;
    s32 t;
    s32 hb;
    s32 mb;

loop:
    switch (g_checkPSState)
    {
    case 0: /* Idle / reset */
        break;

    case 1: /* Init — show opening screen, advance to state 2 */
        func_80051620(1);
        g_checkPSState = 2;
        state = 1;
        break;

    case 2: /* Wait for response on screen 1; on confirm, latch clock mode and show screen 6 */
        state = func_8005144C(1);
        switch (state)
        {
        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = 2;
            break;

        case 0:
            state = 2;
            break;

        case 1:
            g_displayMode = g_clockMode;
            state = 2;
            func_80051620(6);
            g_checkPSState = 3;
            break;

        case -1:
            func_80051620(1);
            state = 2;
            break;

        default:
            state = 2;
            break;
        }

        break;

    case 3: /* Wait for response on screen 6; on confirm, set command byte from display mode and show screen 2 */
        state = func_8005144C(6);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;
        case 1:
            g_CmdBuf[0] = (g_displayMode >= 2) ? 2 : 0;
            func_80051620(2);
            g_checkPSState = 4;
        /* fall through */
        case 0:
            state = 3;
            break;
        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;
        default:
            state = 3;
            break;
        }
        break;

    case 4: /* Wait for response on screen 2; on confirm, decode RTC BCD time into g_timeBuffer (halved) and show screen 0xC */
        state = func_8005144C(2);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 7;
            break;

        case 1:
        {
            h = ((g_RTCTimeBCD[0] >> 4) * 10) + (g_RTCTimeBCD[0] & 0xF);
            m = (((g_RTCTimeBCD[1] >> 4) * 5) * 2) + (g_RTCTimeBCD[1] & 0xF);
            t = ((h * 60) + m) >> 1;
            hb = t / 60;
            mb = t % 60;
            g_timeBuffer[0] = hb;
            g_timeBuffer[1] = mb;
            hb = 7;
            g_timeBuffer[0] = ((g_timeBuffer[0] / 10) << 4) | (g_timeBuffer[0] % 10);
            g_timeBuffer[1] = ((g_timeBuffer[1] / 10) << 4) | (g_timeBuffer[0] % 10);
            func_80051620(0xC);
            state = hb;
            g_checkPSState = 5;
            break;
        }

        case 0:
            state = 7;
            break;
        }

        break;

    case 5: /* Wait for screen 0xC; on confirm button (statusFlag bit 6), fill g_CmdBuf with encoded time and send */
    {
        u8* base;
        state = func_8005144C(0xC);
        base = (u8*)&g_statusFlag;
        switch (state)
        {
        case -1:
            if (!(base[0] & 1))
            {
                g_checkPSState = 1;
                state = -1;
            }
            else if (base[1] & 0x40)
            {
                u8* dst = g_CmdBuf;
                state = 5;
                dst[2] = 0;
                dst[0] = g_timeBuffer[0];
                dst[1] = g_timeBuffer[1];
                func_80051620(3);
                g_checkPSState = 7;
            }
            break;

        case 0:
            state = 5;
            break;

        case 1:
            func_80051620(0xD);
            g_checkPSState = 6;
            state = 5;
            break;

        case -2:
            func_80051620(0);
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
        h = func_8005144C(0xD);
        state = h;
        switch (state)
        {
        case -1:
            func_80051710();
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
            u8* dst = g_CmdBuf;
            dst[2] = 0;
            dst[0] = g_timeBuffer[0];
            dst[1] = g_timeBuffer[1];
            func_80051620(3);
            g_checkPSState = 7;
            state = 6;
            break;
        }

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 6;
            break;
        }

        break;

    case 7: /* Wait for screen 3 response; on confirm, set g_CmdBuf[0]=1 and show screen 5 */
        state = func_8005144C(3);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            g_CmdBuf[0] = (u8)state;
            func_80051620(5);
            g_checkPSState = 8;
        /* fall through */
        case 0:
            state = 7;
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 7;
            break;
        }

        break;

    case 8: /* Wait for screen 5 response; on confirm, record vsync timestamp and advance */
        state = func_8005144C(5);
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
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 8;
            break;
        }

        break;

    case 9: /* Wait 3 vsyncs, then show screen 4 */
        state = 9;
        new_var2 = (g_vsyncTimestamp + 3) < VSync(-1);
        if (new_var2)
        {
            func_80051620(4);
            g_checkPSState = 0xA;
        }
        state = 9;
        break;

    case 10: /* Wait for screen 4 response; on confirm, show screen 7 */
        state = func_8005144C(4);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            func_80051620(7);
            g_checkPSState = 0xB;
        /* fall through */
        case 0:
            state = 0xA;
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xA;
            break;
        }

        break;

    case 11: /* Wait for screen 7 response; on confirm, show screen 8 */
        state = func_8005144C(7);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            func_80051620(8);
            g_checkPSState = 0xC;
        /* fall through */
        case 0:
            state = 0xB;
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xB;
            break;
        }

        break;

    case 12: /* Wait for screen 8 response; on confirm, set g_CmdBuf[0]=4 and show screen 9 */
        state = func_8005144C(8);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            g_CmdBuf[0] = 4;
            func_80051620(9);
            g_checkPSState = 0xD;
        /* fall through */
        case 0:
            state = 0xC;
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xC;
            break;
        }

        break;

    case 13: /* Wait for screen 9 response; on confirm, record vsync timestamp and advance */
        state = func_8005144C(9);
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
            func_80051620(0);
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
            func_80051620(0xA);
            g_checkPSState = 0xF;
        }
        state = 0xE;
        break;

    case 15: /* Wait for screen 0xA response; on confirm, branch on whether RTC hours is nonzero */
        state = func_8005144C(10);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            if (g_RTCTimeBCD[0] != 0)
            {
                func_80051620(0);
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
            func_80051620(0);
            g_checkPSState = 17;
            state = -1;
            break;

        default:
            state = 15;
            break;
        }

        break;

    case 16: /* Wait for screen 0 response */
        state = func_8005144C(0);
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
            func_80051710();
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 17;
            state = -1;
            break;

        default:
            state = 16;
            break;
        }

        break;

    case 17: /* Wait for screen 0 response (retry path); on confirm, restart state machine from state 1 */
        state = func_8005144C(0);
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
            func_80051620(0);
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
        state = func_8005144C(0xB);
        switch (state)
        {
        case -2:
            func_80051620(0);
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
            func_80051620(11);
            g_checkPSState = 18;
        }
        state = 19;
        break;

    default:
        state = 0;
        break;
    }

    if ((arg0 == 0) && (state != 0))
    {
        goto loop;
    }
    return state;
}