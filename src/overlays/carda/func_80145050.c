#include "common.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct
{
    u8 raw[6];
} CardaFileHeaderScratch;

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))

extern s32 D_80165F80;
extern s32 D_80165F38;
extern s32 D_80165F3C;
extern s32 D_80165F7C;
extern s32 D_80165FE8;
extern s32 D_80165FEC;
extern u8 *D_80165FF0;
extern s32 D_80165FF4;
extern s32 D_80165FF8;
extern s32 D_80165FFC;
extern s32 D_80166000;
extern s32 D_80166070;
extern s32 D_80166078;
extern s32 D_801660A0;
extern s32 D_80166104;
extern s32 D_80166118;
extern u8 *D_801663A0;
extern CardaFileHeaderScratch D_801663F8;
extern u8 D_80166440[];
extern s32 D_80166AE0;
extern s32 D_80166B8C;
extern u8 D_80165BB8;
extern u8 D_80165BBD;

extern u8 *D_8012271C;
extern s32 D_801227C4;
extern s32 D_80122988;
extern s32 D_8012298C;
extern s32 D_8012299C;
extern s32 D_801229B0;

extern u8 D_800EC3FA;
extern u8 D_800ECF8C[];
extern const CardaFileHeaderScratch D_801400C4;
extern void *jtbl_801400CC[];
extern void *jtbl_8014012C[];

extern u16 D_8014B03A;
extern u16 D_8014B07A;
extern u16 D_8014B094;
extern u16 D_8014B09C;
extern u16 D_8014B0AC;
extern u16 D_8014B0AE;
extern u16 D_8014B0B0;
extern u16 D_8014B0B6;
extern u16 D_8014B0BA;
extern u16 D_8014B0C2;
extern u16 D_8014B0C6;
extern u16 D_8014B0CA;
extern u16 D_8014B0CC;
extern u16 D_8014B0D6;
extern u16 D_8014B0D8;
extern u16 D_8014B0DA;
extern u16 D_8014B0E6;
extern u16 D_8014B0EC;

s32 func_8001686C(void *);
void func_80016F9C(void *, void *);
s32 func_8001714C(void *, void *, s32);
void func_8001729C(s32);
s32 func_8002054C(s32);
void func_80067F5C(s32);
void func_800A3938(s32, s32);
s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);
void func_800AA02C(void);
s32 func_8014385C(s32, s32 *);
void func_80146694(void);
void func_801466F8(void);
s32 func_80146794(s32, s32 *, s32, s32);
void func_80146CA4(void);
void func_80147100(void);
void func_80149DF4(void);

s32 func_80145050(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 dispatch;
    s32 *state_ptr;
    static void *const keep[] __attribute__((section(".discard"))) = {
        &&dispatch_page, &&return_prim,
        &&case_e9, &&case_ea, &&case_eb, &&case_ec, &&case_ed, &&case_ee, &&case_ef, &&case_f0,
        &&case_f1, &&case_f2, &&case_f3, &&case_f4, &&case_f5, &&case_f6, &&case_f7, &&case_f8,
        &&case_f9, &&case_fa, &&case_fb, &&case_fc, &&case_fd, &&case_fe, &&case_ff, &&case_default
    };

    switch (0)
    {
    case 0:
        if (D_80165F80 & 7)
        {
            dispatch = D_80165FEC - 0xE9;
            if ((u32)dispatch < 0x17)
            {
                goto *jtbl_801400CC[dispatch];
            }
        }

dispatch_page:
        state_ptr = &D_80165FEC;
        dispatch = *state_ptr - 0xE9;
        if ((u32)dispatch >= 0x17)
        {
            goto case_default;
        }
        goto *jtbl_8014012C[dispatch];

case_f8:
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0xE - y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
            goto return_prim;
        }

case_f9:
        if (D_80166078 == 3)
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0BA;
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

case_f6:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A, 0x42), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;

case_ff:
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0AC;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AC, 0x74), 4, x, -y_offset, 2);
            base -= 0x74;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x7A), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            goto return_prim;
        }

case_fa:
        if (D_80166078 == 3)
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0xE - y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
        }
        else
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B03A;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03A, 2), 4, x, -y_offset, 2);
            base -= 2;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x5A), 4, x, 0xE - y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
        }
        goto return_prim;

case_f7:
        if (D_80166078 == 3)
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0x10 - y_offset, 2);
            D_80165FEC = 0xFA;
            goto return_prim;
        }
        else
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B03A;
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

case_fd:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AE, 0x76), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;

case_fb:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0B0, 0x78), 4, -x_offset + 0x90, -y_offset, 2);
        goto return_prim;

case_fc:
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0xE - y_offset, 2);
            prim = func_80146794(prim, ot, 0x90 - x_offset, 0x1C - y_offset);
            goto return_prim;
        }

case_f5:
        {
            s32 x;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0BA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0BA, 0x82), 4, x, -y_offset, 2);
            base -= 0x82;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x84), 4, x, 0x10 - y_offset, 2);
            goto return_prim;
        }

case_f0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0D6, 0x9E), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
case_ef:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0D8, 0xA0), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
case_ee:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0B6, 0x7E), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
case_ed:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AE, 0x76), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
case_ec:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B094, 0x5C), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
case_eb:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B09C, 0x64), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;
case_e9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0E6, 0xAE), 4, -x_offset + 0x90, -y_offset, 2);
        prim = func_80146794(prim, ot, 0x90 - x_offset, 0xE - y_offset);
        goto return_prim;

case_ea:
        {
            s32 x;
            s32 y;
            s32 palette;
            s32 one;
            s32 choice_prim;
            u8 *base;
            u8 *choice_base;
            u8 *choice;
            x = -x_offset;
            base = (u8 *)&D_8014B0C2;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0C2, 0x8A), 4, x + 0x90, -y_offset, 2);
            base -= 0x8A;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xAA), 4, x + 0x90, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xAC), 4, x + 0x90, 0x1C - y_offset, 2);
            {
                u8 *p;
                u8 *choice_base;
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
                choice_prim = func_800A88A0(prim, ot, (void *)g1, a3, x + 0x80, y, one);
                a3 = 4;
                g2 = choice_base[0x38] + ((choice_base[0x39] << 8) + (s32)choice_base);
                if (D_80165FF8 == 0)
                {
                    a3 = 5;
                }
                choice_prim = func_800A88A0(choice_prim, ot, (void *)g2, a3, x + 0x98, y, 0);
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
                func_800AA02C();
                goto return_prim;
            }
            if (D_80122988 & 0x220)
            {
                if (D_80165FF8 != 0)
                {
                    func_800A3938(0x7D, 0x80);
                    D_80165FF8 = D_801660A0;
                    D_80165FEC = 0xF1;
                    func_800AA02C();
                    goto return_prim;
                }
                func_800A3938(0x7E, 0x80);
                D_80165F3C = 0;
                D_80166B8C = func_8002054C(-1);
                D_80166070 = one;
                D_801663A0 = &D_80165BB8;
                D_80165FEC = 0xF4;
            }
            goto return_prim;
        }

found_f4_search:
        D_80165F7C = 1;
        goto done_f4_search_outer;
case_f4:
        {
            s32 i;
            s32 x;
            s32 *slot_index;
            u8 *base;
            u8 *cursor;
            u8 *entries;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0CA;
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
                target_value = *(s32 *)(D_80165FF0 + 0x3B4);
loop_f4_search:
                if (cursor[0x2EF4] != 0)
                {
                    if (*(s32 *)(cursor + 0x2F50) == target_value)
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
done_f4_search:
                ;
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
                slot_index = &D_801227C4;
                *slot_index = 0;
                while (*slot_index < 5 && entries[*slot_index * 0x60 + 0x2EF4] != 0)
                {
                    (*slot_index)++;
                }
                D_8012299C = 5;
                if (*slot_index == 5)
                {
                    s32 *element;
                    D_8012299C = 7;
                    D_8012298C = 0x20;
                    element = &D_80165F80;
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
                do
                {
                    D_801663F8 = D_801400C4;
                } while (0);
                D_801229B0 = *slot_index;
                D_801663F8.raw[2] += (u8)D_801660A0;
                func_80016F9C(&D_801663F8, D_800ECF8C);
                func_8001729C(D_801660A0);
                func_8001686C(&D_801663F8);
                if (D_80165FE8 == 0)
                {
                    s32 *element;
                    D_8012298C = 0x20;
                    element = &D_80165F80;
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
                func_80146CA4();
                goto return_prim;
            }
            D_80165FF8 = 1;
            func_800AA02C();
            goto return_prim;
        }

case_f1:
        prim = func_80146794(prim, ot, 0x90 - x_offset, -y_offset);
        goto return_prim;

case_f2:
        {
            s32 x;
            s32 y;
            s32 y_base;
            s32 a3;
            s32 g1;
            s32 g2;
            s32 hi;
            s32 choice_prim;
            u8 *p;
            u8 *choice_base;
            if (D_80165F3C != 0)
            {
                u8 *base;
                x = -x_offset;
                base = (u8 *)&D_8014B0CC;
                prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0CC, 0x94), 4, x + 0x90, -y_offset, 2);
                base -= 0x94;
                prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x96), 4, x + 0x90, 0xE - y_offset, 2);
                a3 = 4;
                y_base = 0x1C;
            }
            else
            {
                u8 *base;
                x = -x_offset;
                base = (u8 *)&D_8014B0C6;
                prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0C6, 0x8E), 4, x + 0x90, -y_offset, 2);
                base -= 0x8E;
                prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x94), 4, x + 0x90, 0xE - y_offset, 2);
                prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x98), 4, x + 0x90, 0x1C - y_offset, 2);
                a3 = 4;
                y_base = 0x2A;
            }
            y = y_base - y_offset;
            p = &D_800EC3FA;
            hi = p[1] << 8;
            choice_base = p - 0x36;
            g1 = p[0] + (hi + (s32)choice_base);
            if (D_80165FF8 != 0)
            {
                a3 = 5;
            }
            choice_prim = func_800A88A0(prim, ot, (void *)g1, a3, x + 0x80, y, 1);
            a3 = 4;
            g2 = choice_base[0x38] + ((choice_base[0x39] << 8) + (s32)choice_base);
            if (D_80165FF8 == 0)
            {
                a3 = 5;
            }
            choice_prim = func_800A88A0(choice_prim, ot, (void *)g2, a3, x + 0x98, y, 0);
            if (D_80122988 & 0xA000)
            {
                D_80165FF8 ^= 1;
                func_800A3938(0x7D, 0x80);
                D_80122988 = 0;
            }
            prim = choice_prim;
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
            func_800AA02C();
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
            D_801663A0 = &D_80165BBD;
            D_80165FEC = 0xF3;
            goto return_prim;
        }

case_f3:
        {
            s32 x;
            s32 i;
            u8 *base;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0DA;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0DA, 0xA2), 4, x, -y_offset, 2);
            base -= 0xA2;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x7A), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            prim = func_8014385C(prim, ot);
            if (D_80166118 == 0)
            {
                D_8012299C = 4;
                func_800A3938(0x7A, 0x80);
                if (D_801229B0 == 5)
                {
                    { u8 *entry = D_8012271C + D_801227C4 * 0x60; entry[0x2EF4] = 0; }
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
                D_8012298C = 0x20;
                {
                    s32 *element;
                    element = &D_80165F80;
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

case_default:
        {
            s32 x;
            s32 *slot_value;
            u8 *base;
            u8 *filename;
            x = -x_offset + 0x90;
            base = (u8 *)&D_8014B0AC;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0AC, 0x74), 4, x, -y_offset, 2);
            base -= 0x74;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x7A), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            if (D_80166AE0 == 0 && D_80166000 == 0 && (u32)(*D_801663A0 - 6) >= 2U)
            {
                u8 *entry;
                filename = D_800ECF8C;
                slot_value = &D_801660A0;
                entry = D_80166440 + *slot_value * 0x320 + D_80165FF4 * 0x28;
                if (func_8001714C(filename, entry, 0xC) != 0)
                {
                    s32 position;
                    s32 diff;
                    D_80165FF4++;
                    if (D_80165FF4 >= *state_ptr)
                    {
                        if (D_80166078 == 3)
                        {
                            *state_ptr = 0xF8;
                            goto return_prim;
                        }
                        do
                        {
                            D_801663F8 = D_801400C4;
                        } while (0);
                        D_80165F3C = 1;
                        D_801663F8.raw[2] += (u8)*slot_value;
                        func_80016F9C(&D_801663F8, filename);
                        func_80146694();
                        *state_ptr = 0xF2;
                        D_80165FF8 = 1;
                        func_800AA02C();
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
                    *state_ptr = 0xEA;
                }
                else
                {
                    D_80165F3C = 0;
                    D_80166B8C = func_8002054C(-1);
                    D_80166070 = 1;
                    D_801663A0 = &D_80165BB8;
                    *state_ptr = 0xF4;
                }
            }
            goto return_prim;
        }

case_fe:
return_prim:
        return prim;
    }
    return prim;
}
