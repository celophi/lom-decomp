#include "carda_internal.h"

/**
 * @see decomp.me (100%)
 */
s32 func_80145050(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 counter;

    if (D_80165F80[0].attr.word & 7)
    {
        switch (D_80165FEC)
        {
        case 0xE9:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFF:
            return prim;
        }
    }
    switch (D_80165FEC)
    {

    case 0xF8:
    {
        s32 x;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&D_8014B0BA;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
        base -= 0x82;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0xE - y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
        goto return_prim;
    }

    case 0xF9:
        if (D_80166078 == 3)
        {
            s32 x;
            u8* base;
            x = -x_offset + 0x90;
            base = (u8*)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0xE - y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
        }
        else
        {
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0EC, 0xB4), 4, -x_offset + 0x90, -y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        }
        goto return_prim;

    case 0xF6:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A, 0x42), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;

    case 0xFF:
    {
        s32 x;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&D_8014B0AC;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AC, 0x74), 4, x, -y_offset, 2);
        base -= 0x74;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x7A), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
        goto return_prim;
    }

    case 0xFA:
        if (D_80166078 == 3)
        {
            s32 x;
            u8* base;
            x = -x_offset + 0x90;
            base = (u8*)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0xE - y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
        }
        else
        {
            s32 x;
            u8* base;
            x = -x_offset + 0x90;
            base = (u8*)&D_8014B03A;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03A, 2), 4, x, -y_offset, 2);
            base -= 2;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x5A), 4, x, 0xE - y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
        }
        goto return_prim;

    case 0xF7:
        if (D_80166078 == 3)
        {
            s32 x;
            u8* base;
            x = -x_offset + 0x90;
            base = (u8*)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0x10 - y_offset, 2);
            D_80165FEC = 0xFA;
            goto return_prim;
        }
        else
        {
            s32 x;
            u8* base;
            x = -x_offset + 0x90;
            base = (u8*)&D_8014B03A;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03A, 2), 4, x, -y_offset, 2);
            base -= 2;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x60), 4, x, 0x10 - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x62), 4, x, 0x20 - y_offset, 2);
            if ((D_80122988 & 0x220) == 0)
            {
                goto return_prim;
            }
            goto cancel_f2;
        }

    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AE, 0x76), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;

    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0B0, 0x78), 4, -x_offset + 0x90, -y_offset, 2);
        goto return_prim;

    case 0xFC:
    {
        s32 x;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&D_8014B0BA;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
        base -= 0x82;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0xE - y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
        goto return_prim;
    }

    case 0xF5:
    {
        s32 x;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&D_8014B0BA;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
        base -= 0x82;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0x10 - y_offset, 2);
        goto return_prim;
    }

    case 0xF0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0D6, 0x9E), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
    case 0xEF:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0D8, 0xA0), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
    case 0xEE:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0B6, 0x7E), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
    case 0xED:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AE, 0x76), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
    case 0xEC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B094, 0x5C), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
    case 0xEB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B09C, 0x64), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
    case 0xE9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0E6, 0xAE), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;

    case 0xEA:
    {
        s32 x;
        s32 y;
        s32 palette;
        s32 one;
        s32 choice_prim;
        u8* base;
        u8* choice_base;
        u8* choice;
        x = -x_offset;
        base = (u8*)&D_8014B0C2;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0C2, 0x8A), 4, x + 0x90, -y_offset, 2);
        base -= 0x8A;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xAA), 4, x + 0x90, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xAC), 4, x + 0x90, 0x1C - y_offset, 2);
        {
            u8* p;
            u8* choice_base;
            s32 g1;
            s32 g2;
            s32 hi;
            s32 a3;
            y = 0x2A - y_offset;
            p = &D_800EC3FA;
            hi = p[1] << 8;
            choice_base = p - 0x36;
            a3 = 4;
            g1 = p[0] + (hi + (s32)choice_base);
            if (D_80165FF8 != 0)
            {
                a3 = 5;
            }
            one = 1;
            choice_prim = func_800A88A0(prim, ot, (void*)g1, a3, x + 0x80, y, one);
            a3 = 4;
            g2 = choice_base[0x38] + ((choice_base[0x39] << 8) + (s32)choice_base);
            if (D_80165FF8 == 0)
            {
                a3 = 5;
            }
            choice_prim = func_800A88A0(choice_prim, ot, (void*)g2, a3, x + 0x98, y, 0);
        }
        if (D_80122988 & 0xA000)
        {
            D_80165FF8 ^= 1;
            func_800A3938(0x7D, 0x80);
            D_80122988 = 0;
        }
        prim = choice_prim;
        if (D_80122988 & 0x40)
        {
            func_800A3938(0x7D, 0x80);
            D_80165FF8 = D_801660A0;
            D_80165FEC = 0xF1;
            field_reset_input_repeat();
            goto return_prim;
        }
        if (D_80122988 & 0x220)
        {
            if (D_80165FF8 != 0)
            {
                func_800A3938(0x7D, 0x80);
                D_80165FF8 = D_801660A0;
                D_80165FEC = 0xF1;
                field_reset_input_repeat();
                goto return_prim;
            }
            func_800A3938(0x7E, 0x80);
            D_80165F3C = 0;
            D_80166B8C = func_8002054C(-1);
            D_80166070 = one;
            D_801663A0 = D_80165BB8;
            D_80165FEC = 0xF4;
        }
        goto return_prim;
    }

    found_f4_search:
        D_80165F7C = 1;
        goto done_f4_search_outer;
    case 0xF4:
    {
        s32 i;
        s32 x;

        u8* base;
        u8* cursor;
        u8* entries;
        x = -x_offset + 0x90;
        base = (u8*)&D_8014B0CA;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0CA, 0x92), 4, x, -y_offset, 2);
        base -= 0x92;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xA6), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x7A), 4, x, 0x1C - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x2A - y_offset, 2);
        prim = func_8014385C(prim, ot);
        if (D_80166070 != 0)
        {
            goto return_prim;
        }
        func_800A3938(0x7B, 0x80);
        i = 0;
        cursor = D_8012271C;
        D_80165FEC = 0xF2;
        D_80165F7C = 0;
        {
            s32 target_value;
            target_value = *(s32*)(D_80165FF0 + 0x3B4);
        loop_f4_search:
            if (cursor[0x2EF4] != 0)
            {
                if (*(s32*)(cursor + 0x2F50) == target_value)
                {
                    goto found_f4_search;
                }
            }
            i++;
            cursor += 0x60;
            if (i < 5)
            {
                goto loop_f4_search;
            }
        done_f4_search:;
        }
    done_f4_search_outer:
        if (D_80165F7C != 0)
        {
            D_80165FEC = 0xE9;
            D_801663A0 = 0;
            goto return_prim;
        }
        if (D_80166078 == 3)
        {
            entries = D_8012271C;

            D_801227C4 = 0;
            while (D_801227C4 < 5)
            {
                s32 offset = D_801227C4 * 0x60;
                if (*(u8*)((s32)entries + offset + 0x2EF4) == 0)
                {
                    break;
                }
                (D_801227C4)++;
            }
            g_field_card_overlay_mode = 5;
            if (D_801227C4 == 5)
            {
                s32* element;
                g_field_card_overlay_mode = 7;
                g_menu_element_counter = 0x20;
                element = (s32 *)D_80165F80;
                i = 0;
                do
                {
                    *element &= ~7;
                    element += 3;
                    i++;
                } while (i < 8);
                func_80067F5C(8);
                goto return_prim;
            }
            func_80147100();
            func_801466F8();
            D_801663F8 = D_801400C4;
            D_801229B0 = D_801227C4;
            D_801663F8.raw[2] += (u8)D_801660A0;
            func_80016F9C(&D_801663F8, D_800ECF8C);
            func_8001729C(D_801660A0);
            func_8001686C(&D_801663F8);
            if (D_80165FE8 == 0)
            {
                s32* element;
                g_menu_element_counter = 0x20;
                element = (s32 *)D_80165F80;
                counter = 0;
                do
                {
                    *element &= ~7;
                    element += 3;
                    counter++;
                } while (counter < 8);
                func_80067F5C(8);
                goto return_prim;
            }
            func_80146CA4();
            goto return_prim;
        }
        D_80165FF8 = 1;
        field_reset_input_repeat();
        goto return_prim;
    }

    case 0xF1:
        prim = func_80146794(prim, ot, 0x90 - x_offset, -y_offset);
        goto return_prim;

    case 0xF2:
    {
        if (D_80165F3C != 0)
        {
            s32 x;
            s32 y;
            s32 a3;
            s32 g1;
            s32 g2;
            s32 hi;
            s32 choice_prim;
            u8* p;
            u8* choice_base;
            u8* base;
            x = -x_offset;
            base = (u8*)&D_8014B0CC;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0CC, 0x94), 4, x + 0x90, -y_offset, 2);
            base -= 0x94;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x96), 4, x + 0x90, 0xE - y_offset, 2);
            a3 = 4;
            y = 0x1C - y_offset;
            p = &D_800EC3FA;
            hi = p[1] << 8;
            choice_base = p - 0x36;
            g1 = p[0] + (hi + (s32)choice_base);
            if (D_80165FF8 != 0)
            {
                a3 = 5;
            }
            choice_prim = func_800A88A0(prim, ot, (void*)g1, a3, x + 0x80, y, 1);
            a3 = 4;
            g2 = choice_base[0x38] + ((choice_base[0x39] << 8) + (s32)choice_base);
            if (D_80165FF8 == 0)
            {
                a3 = 5;
            }
            choice_prim = func_800A88A0(choice_prim, ot, (void*)g2, a3, x + 0x98, y, 0);
            if (D_80122988 & 0xA000)
            {
                D_80165FF8 ^= 1;
                func_800A3938(0x7D, 0x80);
                D_80122988 = 0;
            }
            prim = choice_prim;
        }
        else
        {
            s32 x;
            s32 y;
            s32 a3;
            s32 g1;
            s32 g2;
            s32 hi;
            s32 choice_prim;
            u8* p;
            u8* choice_base;
            u8* base;
            x = -x_offset;
            base = (u8*)&D_8014B0C6;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0C6, 0x8E), 4, x + 0x90, -y_offset, 2);
            base -= 0x8E;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x94), 4, x + 0x90, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x98), 4, x + 0x90, 0x1C - y_offset, 2);
            a3 = 4;
            y = 0x2A - y_offset;
            p = &D_800EC3FA;
            hi = p[1] << 8;
            choice_base = p - 0x36;
            g1 = p[0] + (hi + (s32)choice_base);
            if (D_80165FF8 != 0)
            {
                a3 = 5;
            }
            choice_prim = func_800A88A0(prim, ot, (void*)g1, a3, x + 0x80, y, 1);
            a3 = 4;
            g2 = choice_base[0x38] + ((choice_base[0x39] << 8) + (s32)choice_base);
            if (D_80165FF8 == 0)
            {
                a3 = 5;
            }
            choice_prim = func_800A88A0(choice_prim, ot, (void*)g2, a3, x + 0x98, y, 0);
            if (D_80122988 & 0xA000)
            {
                D_80165FF8 ^= 1;
                func_800A3938(0x7D, 0x80);
                D_80122988 = 0;
            }
            prim = choice_prim;
        }
        if (D_80122988 & 0x40)
        {
            goto cancel_f2;
        }
        if ((D_80122988 & 0x220) == 0)
        {
            goto return_prim;
        }
        if (D_80165FF8 == 0)
        {
            goto accept_f2;
        }

    cancel_f2:
        func_800A3938(0x7D, 0x80);
        D_80165FEC = 0xF1;
        D_80165FF8 = D_801660A0;
        field_reset_input_repeat();
        goto return_prim;

    accept_f2:
        func_800A3938(0x7E, 0x80);
        D_80165FE8 = 0;
        if (D_80165F3C == 0)
        {
            func_80147100();
            D_801229B0 = D_801227C4;
        }
        else
        {
            D_801229B0 = 5;
        }
        func_80146694();
        D_80166118 = 1;
        D_801663A0 = D_80165BBD;
        D_80165FEC = 0xF3;
        goto return_prim;
    }

    case 0xF3:
    {
        s32 x;
        s32 i;
        u8* base;
        x = -x_offset + 0x90;
        base = (u8*)&D_8014B0DA;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0DA, 0xA2), 4, x, -y_offset, 2);
        base -= 0xA2;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x7A), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
        prim = func_8014385C(prim, ot);
        if (D_80166118 == 0)
        {
            g_field_card_overlay_mode = 4;
            func_800A3938(0x7A, 0x80);
            if (D_801229B0 == 5)
            {
                {
                    u8* entry = D_8012271C + D_801227C4 * 0x60;
                    entry[0x2EF4] = 0;
                }
            }
            else
            {
                func_801466F8();
            }
            if (D_80165FE8 != 0)
            {
                func_80146CA4();
                goto return_prim;
            }
            g_menu_element_counter = 0x20;
            {
                s32* element;
                element = (s32 *)D_80165F80;
                i = 0;
                do
                {
                    *element &= ~7;
                    element += 3;
                    i++;
                } while (i < 8);
            }
            func_80067F5C(8);
        }
        goto return_prim;
    }

    default:
    {
        s32 x;
        s32* slot_value;
        u8* base;
        u8* filename;
        x = -x_offset + 0x90;
        base = (u8*)&D_8014B0AC;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AC, 0x74), 4, x, -y_offset, 2);
        base -= 0x74;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x7A), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
        if (D_80166AE0 == 0 && D_80166000 == 0 && (u32)(*D_801663A0 - 6) >= 2U)
        {
            u8* entry;
            filename = D_800ECF8C;
            slot_value = &D_801660A0;
            entry = (u8 *)D_80166440 + *slot_value * 0x320 + D_80165FF4 * 0x28;
            if (func_8001714C(filename, entry, 0xC) != 0)
            {
                s32 position;
                s32 diff;
                D_80165FF4++;
                if (D_80165FF4 >= D_80165FEC)
                {
                    if (D_80166078 == 3)
                    {
                        D_80165FEC = 0xF8;
                        goto return_prim;
                    }
                    D_801663F8 = D_801400C4;
                    D_80165F3C = 1;
                    D_801663F8.raw[2] += (u8)*slot_value;
                    func_80016F9C(&D_801663F8, filename);
                    func_80146694();
                    D_80165FEC = 0xF2;
                    D_80165FF8 = 1;
                    field_reset_input_repeat();
                    goto return_prim;
                }
                func_80149DF4();
                position = D_80165FF4 * 0xE;
                diff = position - D_80166104;
                if (diff >= 0x4B)
                {
                    D_80165F38 = position - 0x46;
                    D_80165FFC = 4;
                }
                if (diff < 0)
                {
                    D_80165F38 = position;
                    D_80165FFC = 4;
                }
                goto return_prim;
            }
            if (D_80166078 == 3)
            {
                D_80165FF8 = 1;
                D_80165F3C = 0;
                D_80166B8C = func_8002054C(-1);
                D_80166070 = 1;
                D_80165FEC = 0xEA;
            }
            else
            {
                D_80165F3C = 0;
                D_80166B8C = func_8002054C(-1);
                D_80166070 = 1;
                D_801663A0 = D_80165BB8;
                D_80165FEC = 0xF4;
            }
        }
        goto return_prim;
    }

    case 0xFE:
    return_prim:
        return prim;
    }
    return prim;
}

void func_80146694(void)
{
    u8 **base;
    s32 index;
    s32 offset;

    func_800141EC(0x5E2, D_80165FF0);
    func_80013F2C();

    base = &D_8012271C;
    index = D_801227C4;
    offset = index * 0x60 + 0x2EF4;
    func_80016764(D_80165FF0, *base + offset);
}

#define CARDA_RECORD_SIZE 0x60
#define CARDA_RECORD_ARRAY_OFFSET 0x2EF4
#define CARDA_RECORD_GROWTH_OFFSET 0x2F0C

/**
 * @brief Restore the active record and apply its pending growth value.
 */
void func_801466F8(void)
{
    u8 *record;
    u32 growth;
    u32 low_byte;
    u32 updated_growth;

    func_80016E7C(D_80166008,
                  D_8012271C +
                      (D_801227C4 * CARDA_RECORD_SIZE +
                       CARDA_RECORD_ARRAY_OFFSET),
                  CARDA_RECORD_SIZE);

    record = D_8012271C + D_801227C4 * CARDA_RECORD_SIZE;
    growth = *(u32 *)(record + CARDA_RECORD_GROWTH_OFFSET);
    low_byte = growth & 0xFF;
    growth >>= 8;
    growth += D_80165F40;
    growth <<= 8;
    updated_growth = low_byte | growth;
    *(u32 *)(record + CARDA_RECORD_GROWTH_OFFSET) = updated_growth;
    func_800C1230(D_801227C4);
}

s32 func_80146794(s32 prim, s32 *ot, s32 arg2, s32 arg3)
{
    CardaElement *p;
    s32 result;
    s32 i;

    result = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0D2, 0x9A), 4, arg2, arg3, 2);

    if (D_80122988 & 0xA000) {
        D_80165FEC = 0xF1;
        D_801660A0 ^= 1;
        func_800A3938(0x7D, 0x80);
        return result;
    }

    if (D_80122988 & 0x40) {
        switch (D_80166078) {
        case 2:
            g_field_card_overlay_mode = 6;
            break;
        case 3:
            g_field_card_overlay_mode = 7;
            break;
        default:
            g_field_card_overlay_mode = 3;
            break;
        }
        func_800A3938(0x78, 0x80);
        func_80067F28();
        p = D_80165F80;
        for (i = 0; i < 8; i++, p++) {
            if (p->attr.word & 7) {
                p->attr.word = (((p->attr.word & ~7) | 3) & ~0x78) | 0x40;
            }
        }
        return result;
    }

    if (D_80122988 & 0x220) {
        s32 slot;
        func_800A3938(0x7D, 0x80);
        slot = D_801660A0;
        D_801660F8 = 0;
        D_801663A0 = 0;
        D_80165FEC = 0xFF;
        D_80165FFC = 0;
        D_80165F38 = 0;
        D_80166104 = 0;
        D_80165FF4 = 0;
        D_801660FC = 0;
        *(volatile s32 *)&D_801660A0 = slot ^ 1;
        D_801660A0 = slot;
        func_80147C5C();
        func_8014A044();
        func_80149FEC();
        D_80166ADC = 0;
        D_80165FEC = 0xFF;
        D_801663A0 = D_80165B70;
    }

    return result;
}

/**
 * @brief Reset the CARDA choice state and initialize its mode-specific element.
 * @param arg0 Status-dialog state index.
 * @see matching: 100.00%
 */
void func_8014697C(s32 arg0)
{
    func_800A3938(0x78, 0x80);
    field_reset_input_repeat();
    D_80166118 = 0;
    D_80166070 = 0;
    D_801660FC = 0;
    D_80166000 = 0;
    func_80147C5C();
    D_801663A0 = 0;
    D_80165FE4 = arg0;

    if ((u32)(D_80166078 - 2) < 2)
    {
        switch (arg0)
        {
        case 0:
            D_80165FEC = 0xF0;
            break;
        case 1:
            D_80165FEC = 0xEF;
            break;
        case 2:
            D_80165FEC = 0xEE;
            break;
        case 3:
            D_80165FEC = 0xED;
            break;
        case 4:
            D_80165FEC = 0xEC;
            break;
        case 5:
            D_80165FEC = 0xEB;
            break;
        }
        D_801663A0 = 0;
        return;
    }

    D_80165F8C.draw_handler = (void *)func_80146AF0;
    D_80165F8C.attr.f.unk0_3 = 1;
    D_80165F8C.attr.f.state = 1;
    D_80165F8C.attr.f.x = 0x20;
    D_80165F8C.attr.f.unk0_16 = 0x70;
    D_80165F8C.attr4.f.unk4_0 = 1;
    D_80165F8C.attr4.f.y = 0x14;
    SET_ELEM_CODE(&D_80165F8C, 0);
}

s32 func_80146AF0(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    CardaElement *p;
    s32 i;

    switch (D_80165FE4)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B074, 0x3C), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B078, 0x40), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A, 0x42), 4, -arg2 + 0x80, -arg3, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B076, 0x3E), 4, -arg2 + 0x80, -arg3, 2);
        break;
    }
    if (D_80122988 & 0x220)
    {
        g_menu_element_counter = 0x20;
        p = D_80165F80;
        for (i = 0; i < 8; i++)
        {
            p->attr.word &= ~7;
            p++;
        }
        func_80067F5C(8);
        field_reset_input_repeat();
    }
    return prim;
}

static __inline__ CardaElement *claim_element(void)
{
    CardaElement *p;
    s32 i;
    p = D_80165F80;
    for (i = 0; i < 8; i++, p++) {
        if ((p->attr.word & 7) == 0) {
            p->attr.word = (p->attr.word & ~7) | 1;
            return p;
        }
    }
    return D_80165F80;
}

void func_80146CA4(void)
{
    CardaElement *slot;
    g_menu_element_counter = 0x20;
    {
        CardaElement *clear_p;
        s32 clear_i;
        clear_p = D_80165F80;
        clear_i = 0;
        do {
            clear_i++;
            clear_p->attr.word &= ~7;
            clear_p++;
        } while (clear_i < 8);
    }

    D_80165F80[0].attr.f.state = 1;
    slot = claim_element();

    slot->draw_handler = (void *)func_80146EDC;
    slot->attr.f.unk0_3 = 2;
    slot->attr.f.x = 0x20;
    slot->attr.f.unk0_16 = 0x36;
    slot->attr4.f.unk4_0 = 1;
    slot->attr4.f.y = 0x90;
    SET_ELEM_CODE(slot, 0);

    slot = claim_element();
    slot->draw_handler = (void *)func_80146E80;
    slot->attr.f.unk0_3 = 2;
    slot->attr.f.x = 0x20;
    slot->attr.f.unk0_16 = 0x1A;
    slot->attr4.f.unk4_0 = 1;
    slot->attr4.f.y = 0x10;
    D_80165FFC = 0;
    D_80165F38 = 0;
    SET_ELEM_CODE(slot, 0);
    D_80166104 = 0;
    D_80165F80[0].attr.f.state = 0;
}

s32 func_80146E80(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    return func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BE, 0x86), 4, -x_offset + 0x80, -y_offset, 2);
}

s32 func_80146EDC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 *p;
    s32 delta;
    s32 i;
    s32 result;
    s32 j;
    struct { short x, y, w, h; } pos;

    result = arg1;
    i = 0;
    if (D_80165FE8 > 0) {
        do {
            delta = i * 0xE - D_80166104;
            if ((u32)(delta + 0xD) < 0x9D) {
                result = func_800A88A0(result, arg0,
                    (void *)((u8 *)D_8014B4D4 + D_8014B4D4[D_80165F48[i]]),
                    4, -arg2 + 0x80, delta - arg3, 2);
            }
            i++;
        } while (i < D_80165FE8);
    }

    if (D_80165FFC == 0) {
        if (D_80122988 & 0x1000) {
            if (D_80166104 != 0) {
                func_800A3938(0x7D, 0x80);
                D_80165FFC = 4;
                D_80165F38 -= 0xE;
            }
        } else if (D_80122988 & 0x4000) {
            if ((D_80165FE8 * 0xE - D_80166104) >= 0x8D) {
                func_800A3938(0x7D, 0x80);
                D_80165FFC = 4;
                D_80165F38 += 0xE;
            }
        } else if (D_80122988 & 0x220) {
            func_800A3938(0x7E, 0x80);
            g_menu_element_counter = 0x20;
            p = (s32 *)D_80165F80;
            j = 0;
            do {
                *p &= ~7;
                j++;
                p += 3;
            } while (j < 8);
            func_80067F5C(8);
        }
    }
    return result;
}

typedef struct CardaList {
    s32 unk0;
    s32 count;
    u8 entries[0x50];
} CardaList;

/**
 * @brief Scan the active carda list and record entries whose resource rank is
 *        still below the cap, bumping each such resource's rank.
 * @see (100%)
 */
void func_80147100(void)
{
    u8 *base;
    CardaList *list;
    u32 i;
    u8 *p;
    u8 *resource;
    u8 *entry;
    u8 value;

    base = D_80165FF0;
    list = (CardaList *)(base + 0x300);
    D_80165FE8 = 0;
    func_80016E7C(base + 0x358, D_80166008, 0x60);
    i = 0;
    D_80165FE8 = 0;
    D_80165F40 = list->unk0;
    if (list->count != 0)
    {
        do
        {
            p = (u8 *)list + i;
            resource = D_8012271C;
            value = p[8];
            entry = resource + value;
            value = entry[0x25E0];
            if (value < 0x63U)
            {
                entry[0x25E0] = (u8)(value + 1);
                D_80165F48[D_80165FE8] = p[8];
                D_80165FE8++;
            }
            i++;
        } while (i < (u32)list->count);
    }
}
