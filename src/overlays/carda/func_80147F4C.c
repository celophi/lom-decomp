#include "common.h"

typedef struct
{
    u8 raw[6];
} CardaFileHeaderScratch;

typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0x1A];
} CardaLoadScratch;

#define CARD_DIR_BYTES 0x320
#define CARD_ENTRY_BYTES 0x28
#define CARD_ENTRY_PTR() ((void*)((s32)D_80166440 + D_801660A0 * CARD_DIR_BYTES + D_80165FF4 * CARD_ENTRY_BYTES))

extern CardaLoadScratch D_801401C0;

extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern u8 D_800ECFC4[];

extern u8 D_8014BECC[];

extern u8 D_80165B70;
extern u8 D_80165B78;
extern u8 D_80165BAC;
extern s32 D_80165FEC;
extern u8* D_80165FF0;
extern s32 D_80165FF4;
extern s32 D_80166000;
extern s32 D_80166070;
extern s32 D_80166078;
extern s32 D_801660A0;
extern s32 D_801660FC;
extern s32 D_80166118;
extern u8 D_80166120[];
extern u8* D_801663A0;
extern s32 D_801663A4;
extern s32 D_801663A8[];
extern CardaFileHeaderScratch D_801663F8;
extern s32 D_80166438;
extern s32 D_8016643C;
extern u8 D_80166440[];
extern u8 D_80166A80[];
extern s32 D_80166AD0;
extern s32 D_80166AD4;
extern s32 D_80166AD8;
extern s32 D_80166ADC;
extern s32 D_80166AE0;
extern s32 D_80166B88;
extern s32 D_80166B8C;
extern s32 D_80166B90;
extern u8 D_80166BA8[];

s32 func_80016BCC(void*, void*);
s32 func_80016F9C(void*, void*);
s32 func_800170BC(void*, void*, ...);
s32 func_8001724C(s32);
s32 func_8001725C(s32);
s32 func_8001726C(s32, s32, void*);
s32 func_8001727C(s32, s32, void*);
s32 func_8001729C(s32);
s32 func_800172AC(s32);
s32 func_8001680C(void*, s32);
s32 func_8001681C(s32, void*, s32);
s32 func_8001682C(s32, void*, s32);
s32 func_8001683C(s32);
s32 func_8001685C(void*, void*);
s32 func_8001686C(void*);
s32 func_8002054C(s32);
s32 func_80032174(s32, void*, s32*);
s32 func_80033E7C(s32);
s32 func_800342CC(s32);
s32 func_80034648(s32, s32, s32);
void func_80142CA4(void);
s32 func_80143380(void);
void func_801447DC(s32);
void func_801473C0(void*, s32);
void func_8014697C(s32);
s32 func_8014991C(s32);
s32 func_80149A4C(s32);
s32 func_80149DF4(void);
void func_80149FEC(void);
void func_8014A044(void);
s32 func_8014A09C(void);
s32 func_8014A130(void);

static inline void carda_prepare_fixed_names(void)
{
    CardaLoadScratch namebuf;
    memcpy(&namebuf, &D_801401C0, 6);
    ((u8*)&namebuf)[2] += (u8)D_801660A0;
    func_80016F9C(&namebuf, &D_800ECF9C);
    func_8001686C(&namebuf);
    memcpy(&namebuf, &D_801401C0, 6);
    ((u8*)&namebuf)[2] += (u8)D_801660A0;
    func_80016F9C(&namebuf, &D_800ECFB0);
    func_8001686C(&namebuf);
    func_80149FEC();
    func_8014A044();
}

static inline void carda_append_suffix(CardaLoadScratch* buf, CardaLoadScratch* strbuf)
{
    ((u8*)strbuf)[1] = D_80166A80[D_80165FF4 * 4] + 0x30;
    ((u8*)strbuf)[2] = 0;
    func_80016F9C(buf, strbuf);
}

static inline void carda_abort_write(void)
{
    func_80149FEC();
    func_8014A044();
    func_801447DC(0);
}

/** @see decomp.me (100%) */
s32 func_80147F4C(void)
{
    CardaLoadScratch buf;
    CardaLoadScratch strbuf;
    CardaLoadScratch base;
    u8 entryrec[0x80];
    u8 readbuf[0x28];
    s32 wait_attempts;
    s32 poll_shared;
    s32 phase_result;
    s32 status0;
    s32 status1;
    s32 nibble;
    s32 keep_going;
    s32 remaining;
    s8* glyph_out;
    u8* tmpl;
    u8* dst;

    memcpy(&buf, &D_801401C0, 6);
    ((u8*)&buf)[2] += (u8)D_801660A0;
    func_800170BC(&base, &buf);
    phase_result = 1;
    if ((u32)(D_80166078 - 2) < 2U)
    {
        if (D_801663A0 == &D_80165B70)
        {
            D_801663A0 = &D_80165BAC;
        }
    }
    if (D_801663A0 == NULL)
    {
        return phase_result;
    }

    switch (*D_801663A0)
    {
    default:
        goto block_return;
    case 1:
    {
        phase_result = 3;
        func_8001729C(D_801660A0);
        func_8001724C(D_801660A0 * 0x10);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 2:
    {
        poll_shared = func_8014A09C();
        switch (poll_shared)
        {
        case 0:
            D_801663A0++;
            goto block_return;
        case 1:
        case 2:
            goto c2_pos;
        case 3:
            goto c2_eq3;
        default:
            goto block_return;
        }
    c2_pos:
        phase_result = 4;
        D_801660FC = 0;
        D_80165FEC = 0xFD;
        D_801663A0 = D_801663A0 + 1;
        func_80142CA4();
        goto block_return;
    c2_eq3:
        D_80166438 = 0x28;
        {
            s32 index;
            s32 value = -1;
            for (index = 16; index >= 0; index--)
            {
                D_801663A8[index] = value;
            }
        }
        D_80165FEC = 0xFF;
        D_801663A0 = &D_80165B70;
        goto block_return;
    }
    case 25:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll == 0)
        {
            D_801663A0 = D_801663A0 + 1;
            goto block_return;
        }
        if (poll < 0)
        {
            goto block_return;
        }
        if (poll >= 4)
        {
            goto block_return;
        }
        func_801447DC(5);
        goto block_return;
    }
    case 3:
    {
        func_80149FEC();
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 4:
    {
        do
        {
            poll_shared = func_8014A130();
        } while (poll_shared == -1);
        if (poll_shared == 0)
        {
            D_801663A0 = D_801663A0 + 1;
            goto block_return;
        }
        if (poll_shared < 0)
        {
            goto block_return;
        }
        if (poll_shared >= 4)
        {
            goto block_return;
        }
        phase_result = 4;
        goto block_status_fd;
    }
    case 5:
    {
        func_8014A044();
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 6:
    {
        carda_prepare_fixed_names();
        D_80166AE0 = 1;
        if (func_8014991C(D_801660A0) == 0)
        {
            phase_result = 2;
            D_80165FEC = 0xF8;
            D_801663A0 = NULL;
            D_80166AE0 = 0;
            goto block_return;
        }
        D_801663A0 = D_801663A0 + 1;
        wait_attempts = 0;
        do
        {
            if (func_80149A4C(D_801660A0) == 0)
            {
                D_80166AE0 = 0;
                if (D_80165FEC != 0xF8 && D_80165FEC != 0xFA)
                {
                    if (D_80165FEC != 0xF7)
                    {
                        func_80149DF4();
                    }
                    goto block_return;
                }
                goto block_return;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        goto block_return;
    }
    case 8:
    {
        phase_result = 3;
        func_8001729C(D_801660A0);
        func_800172AC(D_801660A0 * 0x10);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 9:
    {
        phase_result = 3;
        func_8001729C(D_801660A0);
        func_8001725C(D_801660A0 * 0x10);
        D_80166AD4 = 0x10;
        D_80166B90 = 0x10;
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 0:
    {
        phase_result = 2;
        D_80166118 = 0;
        goto block_return;
    }
    case 10:
    {
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 11:
    {
        if (D_80166B88 == 0)
        {
            func_80016F9C(&buf, CARD_ENTRY_PTR());
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&buf) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            func_80149FEC();
            func_8014A044();
            func_800170BC(&buf, &base);
        }
        func_800170BC(&strbuf, &D_800ECF9C);
        func_80016F9C(&buf, &strbuf);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&buf, 0x20200);
        if (D_801663A4 == -1)
        {
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                carda_abort_write();
                return phase_result;
            }
            goto block_return;
        }
        func_8001683C(D_801663A4);
        func_800170BC(&D_80166BA8, &buf);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 12:
    {
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_80166BA8, 0x8002);
        func_80149FEC();
        D_80166ADC = 1;
        D_80166B8C = func_8002054C(-1);
        func_8001729C(D_801660A0);
        if (func_8001682C(D_801663A4, D_80165FF0, 0x4000) == -1)
        {
            func_8001683C(D_801663A4);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                carda_abort_write();
                return phase_result;
            }
            goto block_decrement_step;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 13:
    {
        s32 poll;
        s32 outer_count;
        poll = func_8014A09C();
        if (poll != 0)
        {
            if (poll < 0)
            {
                goto block_return;
            }
            if (poll >= 4)
            {
                goto block_return;
            }
            goto c13_nonzero;
        }
        if (D_80166B88 != 0)
        {
            func_800170BC(&buf, &base);
            func_80016F9C(&buf, CARD_ENTRY_PTR());
            wait_attempts = 0;
            func_8001729C(D_801660A0);
            do
            {
                s32 result = func_8001686C(&buf);
                wait_attempts++;
                if (result != 0)
                {
                    break;
                }
            } while (wait_attempts < 0x14);
            func_80149FEC();
            func_8014A044();
        }
        func_800170BC(&buf, &base);
        func_800170BC(&strbuf, &D_800ECF7C);
        glyph_out = (s8*)&strbuf + 0xC;
        remaining = D_80166AD8;
        outer_count = 5;
        wait_attempts = 7;
        keep_going = 0;
        while (outer_count != 0)
        {
            nibble = (remaining >> (wait_attempts * 4)) & 0xF;
            if (nibble != 0 || keep_going != 0)
            {
                func_801473C0(glyph_out, nibble);
                glyph_out += 1;
                outer_count -= 1;
                keep_going = 1;
                remaining -= nibble << (wait_attempts * 4);
            }
            wait_attempts--;
            if (wait_attempts == -1)
            {
                goto hex_done;
            }
            if (wait_attempts == 0)
            {
                keep_going = 1;
            }
        }
    hex_done:
        *glyph_out = 0;
        func_80016F9C(&buf, &strbuf);
        if (func_80143380() != 0)
        {
            *(s8*)&strbuf = 0x2B;
        }
        else
        {
            *(s8*)&strbuf = 0x2D;
        }
        carda_append_suffix(&buf, &strbuf);
        wait_attempts = 0;
        func_8001729C(D_801660A0);
        do
        {
            s32 result = func_8001685C(&D_80166BA8, &buf);
            wait_attempts++;
            if (result != 0)
            {
                break;
            }
        } while (wait_attempts < 0x14);
        wait_attempts = 0;
        do
        {
            if (func_80016BCC(&buf, &readbuf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        if (wait_attempts != 0x14)
        {
            func_8001729C(D_801660A0);
            func_8001727C(D_801660A0 * 0x10, *(s32*)&readbuf[0x20], &entryrec);
            func_8001729C(D_801660A0);
            dst = &entryrec[4];
            tmpl = &D_8014BECC[0];
            wait_attempts = 0;
            do
            {
                wait_attempts = wait_attempts + 1;
                *dst = *tmpl;
                tmpl += 1;
                dst += 1;
            } while (wait_attempts < 0x12);
            if (func_80143380() != 0)
            {
                entryrec[0xC] = 0x81;
                entryrec[0xD] = 0xF4;
            }
            func_8001729C(D_801660A0);
            func_8001726C(D_801660A0 * 0x10, *(s32*)&readbuf[0x20], &entryrec);
            func_8001729C(D_801660A0);
        }
        D_80166118 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        D_80165FEC = 0xFF;
        D_801663A0 = &D_80165B70;
        goto block_return;
    c13_nonzero:
        D_8016643C = D_8016643C - 1;
        if (D_8016643C == 0)
        {
            goto c13_loop103;
        }
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_801663A0 = D_801663A0 - 2;
        goto block_return;
    c13_loop103:
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80166ADC = 0;
        D_80166B8C = func_8002054C(-1);
        carda_abort_write();
        return phase_result;
    }
    case 21:
    {
        D_80166ADC = 1;
        D_80166B8C = func_8002054C(-1);
        if (func_8001714C(&D_800ECFC4, CARD_ENTRY_PTR(), 8) != 0)
        {
            func_80016F9C(&buf, CARD_ENTRY_PTR());
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&buf) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            func_80149FEC();
            func_8014A044();
        }
        func_800170BC(&buf, &base);
        func_80016F9C(&buf, &D_800ECF8C);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&buf, 0x60200);
        if (D_801663A4 == -1)
        {
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                func_8001683C(-1);
                func_801447DC(0);
                return phase_result;
            }
            goto block_return;
        }
        func_8001683C(D_801663A4);
        func_800170BC(&D_80166BA8, &buf);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 22:
    {
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_80166BA8, 0x8002);
        func_8001729C(D_801660A0);
        if (func_8001682C(D_801663A4, D_80165FF0, 0xC000) == -1)
        {
            wait_attempts = 0;
            func_80033E7C(D_801660A0 * 0x10);
            func_8001683C(D_801663A4);
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                carda_abort_write();
                return phase_result;
            }
            goto block_decrement_step;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 23:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll != 0)
        {
            if (poll < 0)
            {
                goto block_return;
            }
            if (poll >= 4)
            {
                goto block_return;
            }
            goto c23_nonzero;
        }
        wait_attempts = 0;
        D_80166118 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        D_801663A0 = &D_80165B70;
        do
        {
            if (func_80016BCC(&D_80166BA8, &readbuf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        if (wait_attempts == 0x14)
        {
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            carda_abort_write();
            return phase_result;
        }
        wait_attempts = 0;
        do
        {
            if (func_80034648(D_801660A0 * 0x10, *(s32*)&readbuf[0x20] / 64, 1) != 0)
            {
                break;
            }
            wait_attempts++;
        } while (wait_attempts < 0x14);
        if (wait_attempts != 0x14)
        {
            return phase_result;
        }
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        carda_abort_write();
        return phase_result;
    c23_nonzero:
        D_8016643C = D_8016643C - 1;
        if (D_8016643C == 0)
        {
            goto c23_loop151;
        }
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_801663A0 = D_801663A0 - 2;
        goto block_return;
    c23_loop151:
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        func_80149FEC();
        func_8014A044();
        func_801447DC(0);
        D_80166ADC = 0;
        D_80166B8C = func_8002054C(-1);
        goto block_return;
    }
    case 15:
    {
        s32 poll;
        poll = func_8014A09C();
        switch (poll)
        {
        case 0:
            D_801663A0++;
            goto block_return;
        case 1:
        case 2:
            goto c15_pos;
        case 3:
            goto c15_eq3;
        default:
            goto block_return;
        }
    c15_pos:
        D_80166B90 = D_80166B90 - 1;
        if (D_80166B90 != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
    block_status_fd:
        D_801660FC = 0;
        D_80165FEC = 0xFD;
        goto block_return;
    c15_eq3:
        D_80166AD4 = D_80166AD4 - 1;
        if (D_80166AD4 == 0)
        {
            goto c15_done;
        }
    block_reissue:
        func_8001729C(D_801660A0);
        func_800172AC(D_801660A0 * 0x10);
        func_8001729C(D_801660A0);
        func_80149FEC();
        func_8001725C(D_801660A0 * 0x10);
        goto block_return;
    c15_done:
        phase_result = 5;
        D_80165FEC = 0xFC;
        D_801663A0 = &D_80165B78;
        goto block_return;
    }
    case 16:
    {
        func_8001729C(D_801660A0);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 17:
    {
        D_80166000 = 1;
        D_801660FC = 0;
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_801663F8, 0x8001);
        if (D_801663A4 != -1)
        {
            func_80149FEC();
            func_8001729C(D_801660A0);
            if (func_8001681C(D_801663A4, &D_80166120, D_80166AD0 != 0 ? 0x280 : 0x80) == -1)
            {
                func_8001683C(D_801663A4);
                return phase_result;
            }
            D_801663A0 = D_801663A0 + 1;
            goto block_return;
        }
        goto block_return;
    }
    case 18:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll == 0)
        {
            D_80166000 = 0;
            D_801660FC = 1;
            D_801663A0 = D_801663A0 + 1;
            func_8001683C(D_801663A4);
            goto block_return;
        }
        if (poll != -1)
        {
            D_80165FEC = 0xFF;
            D_80166000 = 0;
            func_8001683C(D_801663A4);
            D_801663A0 = &D_80165B70;
            goto block_return;
        }
        goto block_return;
    }
    case 19:
    {
        D_80166070 = 1;
        D_80166B8C = func_8002054C(-1);
        D_80166ADC = 1;
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_801663F8, 0x8001);
        func_80149FEC();
        func_8001729C(D_801660A0);
        if (func_8001681C(D_801663A4, D_80165FF0, 0x4000) == -1)
        {
            func_801447DC(1);
            return phase_result;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 20:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll == 0)
        {
            D_80166070 = 0;
            D_801663A0 = D_801663A0 + 1;
            func_8001683C(D_801663A4);
            goto block_return;
        }
        if (poll < 0)
        {
            goto block_return;
        }
        if (poll < 4)
        {
            D_80166B8C = func_8002054C(-1);
            D_80166ADC = 0;
            func_801447DC(1);
            goto block_return;
        }
        goto block_return;
    }
    case 24:
    {
        s32 outer_count = 0;
        do
        {
            wait_attempts = 0;
            do
            {
                if (func_800342CC(D_801660A0 * 0x10) == 1)
                {
                    break;
                }
                func_8002054C(0);
                wait_attempts++;
            } while (wait_attempts < 0x78);
            func_8002054C(0);
            if (wait_attempts != 0x14)
            {
                func_80032174(0, &status0, &status1);
                switch (status1)
                {
                case 0:
                    D_801663A0++;
                    return phase_result;
                case 1:
                    D_80165FEC = 0xFD;
                    D_801663A0 = NULL;
                    return phase_result;
                }
            }
            outer_count++;
        } while (outer_count < 0x14);
        if (D_80165FEC != 0xF6)
        {
            D_80165FEC = 0xF6;
        }
        D_801663A0 = NULL;
        return phase_result;
    }
    case 28:
    {
        D_80166070 = 1;
        D_80166B8C = func_8002054C(-1);
        D_80166ADC = 1;
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_801663F8, 0x8001);
        func_80149FEC();
        func_8001729C(D_801660A0);
        if (func_8001681C(D_801663A4, D_80165FF0, 0x400) == -1)
        {
        block_200:
            func_8014697C(1);
            return phase_result;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 29:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll != 0)
        {
            goto c29_nonzero;
        }
        D_80166070 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        goto block_return;
    c29_nonzero:
        if (poll < 0)
        {
            goto block_return;
        }
        if (poll >= 4)
        {
            goto block_return;
        }
        D_80166ADC = 0;
        goto block_200;
    }
    case 30:
    {
        D_8016643C = 5;
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 26:
    {
        if (D_80166B88 == 0)
        {
            wait_attempts = 0;
            do
            {
                s32 result = func_8001686C(&D_801663F8);
                wait_attempts++;
                if (result != 0)
                {
                    break;
                }
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&buf, &D_800ECFB0);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&buf, 0x60200);
        if (D_801663A4 == -1)
        {
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                func_8001683C(-1);
                wait_attempts = 0;
                do
                {
                    s32 result = func_8001686C(&buf);
                    wait_attempts++;
                    if (result != 0)
                    {
                        break;
                    }
                } while (wait_attempts < 0x14);
                func_8014697C(0);
                return phase_result;
            }
            goto block_return;
        }
        func_8001683C(D_801663A4);
        func_800170BC(&D_80166BA8, &buf);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_80166BA8, 0x8002);
        func_80149FEC();
        D_80166B8C = func_8002054C(-1);
        D_80166ADC = 1;
        func_8001729C(D_801660A0);
        if (func_8001682C(D_801663A4, D_80165FF0, 0xC000) == -1)
        {
            func_8001683C(D_801663A4);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                func_8014697C(0);
                return phase_result;
            }
            goto block_return;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 27:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll != 0)
        {
            if (poll < 0)
            {
                goto block_return;
            }
            if (poll >= 4)
            {
                goto block_return;
            }
            goto c27_nonzero;
        }
        if (D_80166B88 != 0)
        {
            func_8001729C(D_801660A0);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_801663F8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(D_801660A0);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(&D_80166BA8, &D_801663F8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80166118 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        D_801663A0 = &D_80165B70;
        goto block_return;
    c27_nonzero:
        D_8016643C = D_8016643C - 1;
        if (D_8016643C == 0)
        {
            func_8001683C(D_801663A4);
            D_80166ADC = 0;
            func_8014697C(0);
            return phase_result;
        }
        func_8001683C(D_801663A4);
        goto block_decrement_step;
    }
    }

block_decrement_step:
    D_801663A0--;
block_return:
    return phase_result;
}
