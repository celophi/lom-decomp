#include "checkps.h"

/**
 * 68% match with GNU AS
 * Note that this could be probably FUNCTIONALLY INCORRECT!
 */
s32 func_80050B14(s32 arg0)
{
    int new_var2;
    u8* new_var;
    int new_var7;
    u8* new_var3;
    s32 new_var5;
    u8* new_var4;
    volatile int new_var6;
    unsigned int state;
    new_var4 = &D_800810B8[1];
    {
        new_var7 = 0;
        do
        {
        } while (0);
        switch (D_8005CFE8)
        {
        case 0:
            state = 0;
            break;

        case 1:
            func_80051620(1);
            D_8005CFE8 = 2;
            state = 1;
            break;

        case 2:
            state = func_8005144C(1);
            if (state == (-1))
            {
                func_80051620(1);
                state = 2;
                do
                {
                } while (0);
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 2;
                }
                else if (state == 1)
                {
                    D_800810B4 = D_8005CFE2;
                    func_80051620(6);
                    D_8005CFE8 = 3;
                    state = 2;
                }
                else
                {
                    state = 2;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = 2;
            }
            break;

        case 3:
            state = func_8005144C(6);
            if (state != -1)
            {
                if (state >= 0)
                {
                    if (state == 0)
                    {
                        state = 3;
                    }
                    else
                    {
                        if (state == 1)
                        {
                            D_8005CFD8[0] = (D_800810B4 >= 2) ? (2) : (0);
                            func_80051620(2);
                            D_8005CFE8 = 4;
                        }
                        state = 3;
                    }
                }
                else if (state == (-2))
                {
                    func_80051620(0);
                    D_8005CFE8 = 0x11;
                    state = -1;
                }
            }
            else
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            break;
            new_var = &D_800810B8[1];

        case 4:
            state = func_8005144C(2);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 7;
                }
                else if (state == 1)
                {
                    s32 h = ((D_8005CFE1[0] >> 4) * 10) + (D_8005CFE1[0] & 0xF);
                    s32 m = (((D_8005CFE1[1] >> 4) * 5) * 2) + (D_8005CFE1[1] & 0xF);
                    s32 t = ((h * 60) + m) >> 1;
                    u8 hb = t / 60;
                    u8 mb = t % 60;
                    D_800810B8[0] = hb;
                    D_800810B8[1] = mb;
                    hb = 7;
                    D_800810B8[new_var7] = ((D_800810B8[0] / 10) << 4) | (D_800810B8[0] % 10);
                    D_800810B8[1] = (((*new_var4) / 10) << 4) | ((*new_var) % 10);
                    func_80051620(0xC);
                    state = hb;
                }
                else
                {
                    state = 7;
                }
                D_8005CFE8 = 5;
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 5:
            state = func_8005144C(0xC);
            new_var3 = &D_8005CFE0.unk1;
            new_var4 = D_8005CFE0.unk0;
            if (state == (-1))
            {
                if (!(new_var4[0] & 1))
                {
                    D_8005CFE8 = 1;
                    state = -1;
                }
                else if ((*new_var3) & 0x40)
                {
                    D_8005CFD8[2] = 0;
                    D_8005CFD8[0] = D_800810B8[0];
                    D_8005CFD8[1] = D_800810B8[1];
                    func_80051620(3);
                    D_8005CFE8 = 7;
                    state = 5;
                }
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 5;
                    state = 5;
                }
                else if (state == 1)
                {
                    func_80051620(0xD);
                    D_8005CFE8 = 6;
                    state = 5;
                }
                else
                {
                    state = 5;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 6:
            state = func_8005144C(0xD);
            if (state == (-1))
            {
                func_80051710();
                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 6;
                }
                else if (state == 1)
                {
                    D_8005CFD8[2] = 0;
                    state = D_800810B8[1];
                    D_8005CFD8[0] = D_800810B8[0];
                    do
                    {
                    } while (0);
                    D_8005CFD8[1] = state;
                    func_80051620(3);
                    D_8005CFE8 = 7;
                    state = 6;
                }
                else
                {
                    state = 6;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 7:
            state = func_8005144C(3);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 7;
                }
                else if (state == 1)
                {
                    D_8005CFD8[0] = (u8)state;
                    func_80051620(5);
                    D_8005CFE8 = 8;
                    state = 7;
                }
                else
                {
                    state = 7;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 8:
            state = func_8005144C(5);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 8;
                }
                else if (state == 1)
                {
                    D_800810B0 = VSync(-1);
                    D_8005CFE8 = 9;
                    state = 8;
                }
                else
                {
                    state = 8;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 9:
            state = 9;
            new_var2 = (D_800810B0 + 3) < VSync(-1);
            if (new_var2)
            {
                func_80051620(4);
                D_8005CFE8 = 0xA;
            }
            break;

        case 10:
            state = func_8005144C(4);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if (state >= 0)
            {
                if ((unsigned int)(state == 0))
                {
                    state = 0xA;
                }
                else if (state == 1)
                {
                    func_80051620(7);
                    D_8005CFE8 = 0xB;
                    state = 0xA;
                }
                else
                {
                    state = 0xA;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 11:
            new_var5 = func_8005144C(7);
            state = new_var5;
            if (state == -1)
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 0xB;
                }
                else if (state == 1)
                {
                    func_80051620(8);
                    D_8005CFE8 = 0xC;
                    state = 0xB;
                }
                else
                {
                    state = 0xB;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 12:
            state = func_8005144C(8);
            if (state == (-1))
            {
                D_8005CFE8 = 1;

                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 0xC;
                }
                else if (state == 1)
                {
                    D_8005CFD8[0] = 4;
                    func_80051620(9);
                    D_8005CFE8 = 0xD;
                    state = 0xC;
                }
                else
                {
                    state = 0xC;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 13:
            state = func_8005144C(9);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 0xD;
                }
                else if (state == 1)
                {
                    D_800810B0 = VSync(-1);
                    D_8005CFE8 = 0xE;
                    state = 0xD;
                }
                else
                {
                    state = 0xD;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;
            state = 0xE;

        case 14:
            if (D_800810B0 + 0xC8 < VSync(-1))
            {
                D_8005CFD8[0] = 5;
                func_80051620(0xA);
                D_8005CFE8 = 0xF;
            }
            state = 0xE;
            break;

        case 15:
            state = func_8005144C(0xA);
            do
            {
                if (state == (-1))
                {
                    D_8005CFE8 = 1;
                    state = -1;
                }
                else if (state >= 0)
                {
                    if (state == 0)
                    {
                        state = 0xF;
                    }
                    else
                    {
                        if (state == 1)
                        {
                            if (D_8005CFE1[0] != 0)
                            {
                                func_80051620(0);
                                D_8005CFE8 = 0x10;
                            }
                            else
                            {
                                D_800810B0 = VSync(-1);
                                D_8005CFE8 = 0x13;
                            }
                            state = 0xF;
                        }
                        else
                        {
                        }
                        state = 0xF;
                    }
                }
                else if (state == (-2))
                {
                    func_80051620(0);
                    D_8005CFE8 = 0x11;
                    state = -1;
                }
            } while (0);
            break;

        case 16:
            state = func_8005144C(0);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if (state >= 0)
            {
                if (state == 0)
                {
                    state = 0x10;
                }
                else if (state == 1)
                {
                    func_80051710();
                    if ((state && state) && state)
                    {
                    }
                    state = 0x10;
                }
                else
                {
                    state = 0x10;
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 17:
            state = func_8005144C(0);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else
            {
                new_var2 = state;
                if (new_var2 >= 0)
                {
                    if (new_var2 == 0)
                    {
                        state = 0x10;
                        do
                        {
                        } while (0);
                    }
                    else if (state == 1)
                    {
                        D_8005CFE8 = (u32)state;
                        state = 0x10;
                    }
                    else
                    {
                        state = 0x10;
                    }
                }
                else
                {
                    {
                        func_80051620(0);
                        state = 0x10;
                    }
                }
            }
            break;

        case 18:
            state = func_8005144C(0xB);
            if (state == (-1))
            {
                D_8005CFE8 = 1;
                state = -1;
            }
            else if ((state + 1) >= (0 + 1))
            {
                if (state == 0)
                {
                    state = 0x12;
                }
                else
                {
                    do
                    {
                        if (state == 1)
                        {
                            D_8005CFE8 = 0;
                            state = 0x12;
                        }
                        else
                        {
                            state = 0x12;
                        }
                    } while (0);
                }
            }
            else if (state == (-2))
            {
                func_80051620(0);
                D_8005CFE8 = 0x11;
                state = -1;
            }
            break;

        case 19:
            do
            {
                state = 0x13;
                if (inline_fn(D_800810B0 + 0xA, VSync(-1)))
                {
                    func_80051620(0xB);
                    D_8005CFE8 = 0x12;
                }
                break;
            } while (0);

        default:
            state = 0;
            break;
        }
    }
    new_var6 = state;
    return state;
}