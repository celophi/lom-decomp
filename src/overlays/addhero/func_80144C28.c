#include "common.h"

typedef struct {
    s32 unk0;
    s16 unk4;
    u8 pad[0x62];
} AddheroLoadScratch;

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} AddheroFileHeaderScratch;

typedef struct {
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} AddheroFileHeader;

extern AddheroFileHeader D_80140090;
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern s32 D_801609A4;
extern s32 D_801609A8;
extern s32 D_801609AC;
extern s32 D_801609B8;
extern s32 D_80160934;
extern s32 D_8016093C;
extern u8 *D_80165488;
extern s32 D_8016548C;
extern s32 D_80165490[];
extern s32 D_80165520;
extern s32 D_80164A40;
extern s32 D_80164A4C;
extern s32 D_80164A48;
extern s32 D_80164A50;
extern s32 D_80164A54;
extern s32 D_80164A58;
extern s32 D_80164A60;
extern s32 D_80164B08;
extern s32 D_80164B1C;
extern u8 D_80164B20[];
extern u8 D_80164B60[];
extern s32 D_80165200;
extern u8 D_80165208[];
extern u8 D_801654E0[];
extern u8 D_801609F0[];
extern u8 D_80160574[];
extern u8 D_8016057C[];

s32 func_80016F9C(void *, void *);
s32 func_8001680C(void *, s32);
s32 func_8001681C(s32, void *, s32);
s32 func_8001682C(s32, void *, s32);
s32 func_8001683C(s32);
s32 func_8001685C(void *, void *);
s32 func_8001686C(void *);
s32 func_800170BC(void *, void *, ...);
s32 func_8001724C(s32);
s32 func_8001725C(s32);
s32 func_8001729C(s32);
s32 func_800172AC(s32);
s32 func_8002054C(s32);
s32 func_80032174(s32, void *, s32 *);
s32 func_800342CC(s32);
s32 func_80145B4C(s32);
s32 func_80145C34(s32);
void func_80145E14(void);
void func_80145FC0(void);
void func_80146018(void);
s32 func_80146070(void);
s32 func_80146104(void);
void func_80142B1C(s32);
void func_80142C08(s32);

static inline void addhero_probe_render_two(void)
{
    AddheroFileHeaderScratch p;

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_801609A8;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

s32 func_80144C28(void)
{
    AddheroLoadScratch buf;
    s32 status0;
    s32 status1;
    s32 phase_result;
    s32 wait_attempts;
    s32 poll_result;
    s32 poll_result20;
    s32 rank_index;
    s32 rank_value;

    memcpy(&buf, &D_80140090, 6);
    phase_result = 1;
    ((u8 *)&buf)[2] += *(u8 *)&D_801609A8;

    if (D_80165488 == NULL)
    {
        goto block_return;
    }

    switch (*D_80165488)
    {
    case 1:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_8001724C(D_801609A8 * 0x10);
        D_80165488 = D_80165488 + 1;
        break;

    case 2:
        poll_result = func_80146070();
        if (poll_result >= 3)
        {
            goto c2_ge3;
        }
        if (poll_result > 0)
        {
            goto c2_pos;
        }
        if (poll_result == 0)
        {
            goto c2_increment;
        }
        break;
    c2_ge3:
        if (poll_result == 3)
        {
            goto c2_eq3;
        }
        break;
    c2_increment:
        D_80165488 = D_80165488 + 1;
        break;
    c2_pos:
        phase_result = 4;
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        D_80165488 = D_80165488 + 1;
        break;
    c2_eq3:
        D_80165520 = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            D_80165490[rank_index] = rank_value;
        }
        D_801609A4 = 0xFF;
        D_80165488 = D_80160574;
        break;

    case 3:
        func_80145FC0();
        D_80165488 = D_80165488 + 1;
        break;

    case 4:
        do
        {
            poll_result = func_80146104();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            D_80165488 = D_80165488 + 1;
            break;
        }
        if (poll_result < 0)
        {
            break;
        }
        if (poll_result >= 4)
        {
            break;
        }
        phase_result = 4;
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        break;

    case 5:
        func_80146018();
        D_80165488 = D_80165488 + 1;
        break;

    case 6:
        addhero_probe_render_two();
        D_80164A60 = 1;
        if (func_80145B4C(D_801609A8) == 0)
        {
            phase_result = 2;
            D_80165488 = NULL;
            D_801609A4 = 0xF8;
            D_80164A60 = 0;
            break;
        }
        wait_attempts = 0;
        D_80165488 = D_80165488 + 1;
        do
        {
            if (func_80145C34(D_801609A8) == 0)
            {
                if (D_8016093C != 0)
                {
                    D_801609AC = 0;
                }
                D_80164A60 = 0;
                if (D_801609A4 == 0xF8)
                {
                    break;
                }
                if (D_801609A4 == 0xFA)
                {
                    break;
                }
                func_80145E14();
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        break;

    case 8:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_800172AC(D_801609A8 * 0x10);
        D_80165488 = D_80165488 + 1;
        break;

    case 9:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_8001725C(D_801609A8 * 0x10);
        D_80164A58 = 0x10;
        D_80164B08 = 0x10;
        D_80165488 = D_80165488 + 1;
        break;

    case 0:
        phase_result = 2;
        D_80165200 = 0;
        break;

    case 10:
        func_80016F9C(&buf, D_80164B60 + (D_801609A8 * 0x320) + (D_801609AC * 0x28));
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&buf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80165488 = D_80165488 + 1;
        break;

    case 15:
        poll_result = func_80146070();
        if (poll_result >= 3)
        {
            goto c15_ge3;
        }
        if (poll_result > 0)
        {
            goto c15_pos;
        }
        if (poll_result == 0)
        {
            goto c15_increment;
        }
        break;
    c15_ge3:
        if (poll_result == 3)
        {
            goto c15_eq3;
        }
        break;
    c15_increment:
        D_80165488 = D_80165488 + 1;
        break;
    c15_pos:
        D_80164B08 = D_80164B08 - 1;
        if (D_80164B08 != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        break;
    c15_eq3:
        D_80164A58 = D_80164A58 - 1;
        if (D_80164A58 == 0)
        {
            goto c15_d70zero;
        }
    block_reissue:
        func_8001729C(D_801609A8);
        func_800172AC(D_801609A8 * 0x10);
        func_8001729C(D_801609A8);
        func_8001725C(D_801609A8 * 0x10);
        break;
    c15_d70zero:
        phase_result = 5;
        D_801609A4 = 0xFC;
        D_80165488 = D_8016057C;
        break;

    case 16:
        do
        {
            poll_result = func_80146104();
        } while (poll_result == -1);
        D_80165488 = D_80165488 + 1;
        break;

    case 17:
        D_80164A40 = 1;
        D_801609B8 = 0;
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_801654E0, 0x8001);
        if (D_8016548C == -1)
        {
            break;
        }
        func_80145FC0();
        func_8001729C(D_801609A8);
        if (func_8001681C(D_8016548C, D_80165208,
                           D_80164A50 != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(D_8016548C);
            break;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 18:
        if (D_80164A40 != 0)
        {
            poll_result = func_80146070();
            if (poll_result == 0)
            {
                D_80164A40 = 0;
                D_801609B8 = 1;
                func_8001683C(D_8016548C);
                break;
            }
            if (poll_result == -1)
            {
                break;
            }
            func_8001683C(D_8016548C);
            D_801609A4 = 0xFF;
            D_80165488 = D_80160574;
        }
        else
        {
            D_80165488 = D_80165488 + 1;
        }
        break;

    case 19:
        D_80160934 = 1;
        D_80164A54 = func_8002054C(-1);
        D_80164A4C = 1;
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_801654E0, 0x8001);
        func_80145FC0();
        func_8001729C(D_801609A8);
        if (func_8001681C(D_8016548C, D_801609F0, 0x4000) == -1)
        {
            func_8001683C(D_8016548C);
            D_80164A48 = D_80164A48 - 1;
            if (D_80164A48 == 0)
            {
            block_dialog_read:
                func_80142B1C(1);
                break;
            }
            break;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 20:
        poll_result20 = func_80146070();
        if (poll_result20 == 0)
        {
            D_80160934 = 0;
            D_80165488 = D_80165488 + 1;
            func_8001683C(D_8016548C);
            break;
        }
        if (poll_result20 < 0)
        {
            break;
        }
        if (poll_result20 >= 4)
        {
            break;
        }
        func_8001683C(D_8016548C);
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_read;
        }
        D_80165488 = D_80165488 - 1;
        break;

    case 24:
        wait_attempts = 0;
        do
        {
            if (func_800342CC(D_801609A8 * 0x10) == 1)
            {
                break;
            }
            func_8002054C(0);
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        if (wait_attempts != 0x14)
        {
            func_80032174(0, &status0, &status1);
            if (status1 == 0)
            {
                D_80165488 = D_80165488 + 1;
                break;
            }
        }
        func_80142B1C(3);
        break;

    case 30:
        D_80164A48 = 5;
        D_80165488 = D_80165488 + 1;
        break;

    case 27:
        D_80160934 = 1;
        D_80164A54 = func_8002054C(-1);
        D_80164A4C = 1;
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_801654E0, 0x8001);
        func_80145FC0();
        func_8001729C(D_801609A8);
        if (func_8001681C(D_8016548C, D_801609F0, 0x4000) == -1)
        {
            func_8001683C(D_8016548C);
            D_80164A48 = D_80164A48 - 1;
            if (D_80164A48 == 0)
            {
            block_dialog_write_read:
                func_80142C08(1);
                break;
            }
            break;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 28:
        poll_result20 = func_80146070();
        if (poll_result20 == 0)
        {
            D_80160934 = 0;
            D_80165488 = D_80165488 + 1;
            func_8001683C(D_8016548C);
            break;
        }
        if (poll_result20 < 0)
        {
            break;
        }
        if (poll_result20 >= 4)
        {
            break;
        }
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_write_read;
        }
        goto block_close_decrement;

    case 25:
        if (D_80164B1C == 0)
        {
            func_8001729C(D_801609A8);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_801654E0) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&buf, D_800ECF9C);
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(&buf, 0x20200);
        if (D_8016548C != -1)
        {
            goto block_write_opened;
        }
        func_8001683C(-1);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&buf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
    block_write_retry:
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
        block_dialog_write:
            func_80142C08(0);
            break;
        }
        break;

    block_write_opened:
        func_8001683C(D_8016548C);
        func_800170BC(D_80164B20, &buf);
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_80164B20, 0x8002);
        func_80145FC0();
        D_80164A54 = func_8002054C(-1);
        D_80164A4C = 1;
        func_8001729C(D_801609A8);
        if (func_8001682C(D_8016548C, D_801609F0, 0x4000) == -1)
        {
            func_8001683C(D_8016548C);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164B20) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            goto block_write_retry;
        }
        D_80165488 = D_80165488 + 1;
        break;

    case 26:
        poll_result20 = func_80146070();
        if (poll_result20 != 0)
        {
            if (poll_result20 < 0)
            {
                break;
            }
            if (poll_result20 >= 4)
            {
                break;
            }
            goto block_case26_retry;
        }
        if (D_80164B1C != 0)
        {
            func_8001729C(D_801609A8);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_801654E0) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(D_801609A8);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(D_80164B20, D_801654E0) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80165200 = 0;
        D_80165488 = D_80165488 + 1;
        func_8001683C(D_8016548C);
        break;

    block_case26_retry:
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_write;
        }
        goto block_close_decrement;

    default:
        break;
    }

    goto block_return;

block_close_decrement:
    func_8001683C(D_8016548C);
    D_80165488 = D_80165488 - 1;

block_return:
    return phase_result;
}
