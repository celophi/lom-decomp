#include "checkps.h"

s32 D_800810B0 = 0;

s32 D_800810B4 = 0;

u8 D_800810B8[2] = {0};

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
    case 0:
        state = 0;
        break;

    case 1:
        func_80051620(1);
        g_checkPSState = 2;
        state = 1;
        break;

    case 2:
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
            D_800810B4 = D_8005CFE2;
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

    case 3:
        state = func_8005144C(6);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;
        case 1:
            gCmdBuf[0] = (D_800810B4 >= 2) ? 2 : 0;
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

    case 4:
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
            h = ((D_8005CFE1[0] >> 4) * 10) + (D_8005CFE1[0] & 0xF);
            m = (((D_8005CFE1[1] >> 4) * 5) * 2) + (D_8005CFE1[1] & 0xF);
            t = ((h * 60) + m) >> 1;
            hb = t / 60;
            mb = t % 60;
            D_800810B8[0] = hb;
            D_800810B8[1] = mb;
            hb = 7;
            D_800810B8[0] = ((D_800810B8[0] / 10) << 4) | (D_800810B8[0] % 10);
            D_800810B8[1] = ((D_800810B8[1] / 10) << 4) | (D_800810B8[0] % 10);
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

    case 5:
    {
        u8* base;
        state = func_8005144C(0xC);
        base = (u8*)&D_8005CFE0;
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
                u8* dst = gCmdBuf;
                state = 5;
                dst[2] = 0;
                dst[0] = D_800810B8[0];
                dst[1] = D_800810B8[1];
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

    case 6:
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
            u8* dst = gCmdBuf;
            dst[2] = 0;
            dst[0] = D_800810B8[0];
            dst[1] = D_800810B8[1];
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

    case 7:
        state = func_8005144C(3);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            gCmdBuf[0] = (u8)state;
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

    case 8:
        state = func_8005144C(5);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            D_800810B0 = VSync(-1);
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

    case 9:
        state = 9;
        new_var2 = (D_800810B0 + 3) < VSync(-1);
        if (new_var2)
        {
            func_80051620(4);
            g_checkPSState = 0xA;
        }
        state = 9;
        break;

    case 10:
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

    case 11:
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

    case 12:
        state = func_8005144C(8);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            gCmdBuf[0] = 4;
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

    case 13:
        state = func_8005144C(9);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            D_800810B0 = VSync(-1);
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

    case 14:
        new_var2 = VSync(-1);
        if ((D_800810B0 + 0xC8) < new_var2)
        {
            gCmdBuf[0] = 5;
            func_80051620(0xA);
            g_checkPSState = 0xF;
        }
        state = 0xE;
        break;

    case 15:
        state = func_8005144C(0xA);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 1:
            if (D_8005CFE1[0] != 0)
            {
                func_80051620(0);
                g_checkPSState = 0x10;
            }
            else
            {
                D_800810B0 = VSync(-1);
                g_checkPSState = 0x13;
            }
        /* fall through */
        case 0:
            state = 0xF;
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0xF;
            break;
        }

        break;

    case 16:
        state = func_8005144C(0);
        switch (state)
        {
        case -1:
            g_checkPSState = 1;
            state = -1;
            break;

        case 0:
            state = 0x10;
            break;

        case 1:
            state = 0x10;
            func_80051710();
            break;

        case -2:
            func_80051620(0);
            g_checkPSState = 0x11;
            state = -1;
            break;

        default:
            state = 0x10;
            break;
        }

        break;

    case 17:
        state = func_8005144C(0);
        switch (state)
        {
        case 1:
            g_checkPSState = (u32)state;
            state = 0x10;
            break;

        case -1:
            g_checkPSState = 1;
            state = -1;
            break;
        case -2:
            func_80051620(0);
            state = 0x10;
            break;

        case 0:
            state = 0x10;
            break;

        default:
            state = 0x10;
            break;
        }

        break;

    case 18:
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

    case 19:
        new_var2 = VSync(-1);
        if ((D_800810B0 + 0xA) < new_var2)
        {
            func_80051620(0xB);
            g_checkPSState = 0x12;
        }
        state = 0x13;
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