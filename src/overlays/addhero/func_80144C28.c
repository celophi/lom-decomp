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
extern void *jtbl_80140098[];
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
    s32 dispatch;
    static void *const keep[] = {
        &&cl_case_0, &&cl_case_1, &&cl_case_2, &&cl_case_3,
        &&cl_case_4, &&cl_case_5, &&cl_case_6, &&block_return,
        &&cl_case_8, &&cl_case_9, &&cl_case_10, &&block_return,
        &&block_return, &&block_return, &&block_return, &&cl_case_15,
        &&cl_case_16, &&cl_case_17, &&cl_case_18, &&cl_case_19,
        &&cl_case_20, &&block_return, &&block_return, &&block_return,
        &&cl_case_24, &&cl_case_25, &&cl_case_26, &&cl_case_27,
        &&cl_case_28, &&block_return, &&cl_case_30
    };

    memcpy(&buf, &D_80140090, 6);
    do
    {
        phase_result = 1;
    } while (0);
    ((u8 *)&buf)[2] += *(u8 *)&D_801609A8;

    if (D_80165488 == NULL)
    {
        goto block_return;
    }

    switch (0)
    {
    case 0:
        dispatch = *D_80165488;
        if ((u32)dispatch >= 0x1F)
        {
            goto block_return;
        }
        goto *jtbl_80140098[dispatch];

    cl_case_1:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_8001724C(D_801609A8 * 0x10);
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_2:
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
        goto block_return;
    c2_ge3:
        if (poll_result == 3)
        {
            goto c2_eq3;
        }
        goto block_return;
    c2_increment:
        D_80165488 = D_80165488 + 1;
        goto block_return;
    c2_pos:
        do
        {
            phase_result = 4;
        } while (0);
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        D_80165488 = D_80165488 + 1;
        goto block_return;
    c2_eq3:
        D_80165520 = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            D_80165490[rank_index] = rank_value;
        }
        D_801609A4 = 0xFF;
        D_80165488 = D_80160574;
        goto block_return;

    cl_case_3:
        func_80145FC0();
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_4:
        do
        {
            poll_result = func_80146104();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            D_80165488 = D_80165488 + 1;
            goto block_return;
        }
        if (poll_result < 0)
        {
            goto block_return;
        }
        if (poll_result >= 4)
        {
            goto block_return;
        }
        phase_result = 4;
        goto block_status_fd;

    cl_case_5:
        func_80146018();
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_6:
        addhero_probe_render_two();
        D_80164A60 = 1;
        if (func_80145B4C(D_801609A8) == 0)
        {
            phase_result = 2;
            D_80165488 = NULL;
            D_801609A4 = 0xF8;
            D_80164A60 = 0;
            goto block_return;
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
                    goto block_return;
                }
                if (D_801609A4 == 0xFA)
                {
                    goto block_return;
                }
                func_80145E14();
                goto block_return;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        goto block_return;

    cl_case_8:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_800172AC(D_801609A8 * 0x10);
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_9:
        phase_result = 3;
        func_8001729C(D_801609A8);
        func_8001725C(D_801609A8 * 0x10);
        D_80164A58 = 0x10;
        D_80164B08 = 0x10;
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_0:
        phase_result = 2;
        D_80165200 = 0;
        goto block_return;

    cl_case_10:
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
        goto block_return;

    cl_case_15:
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
        goto block_return;
    c15_ge3:
        if (poll_result == 3)
        {
            goto c15_eq3;
        }
        goto block_return;
    c15_increment:
        D_80165488 = D_80165488 + 1;
        goto block_return;
    c15_pos:
        D_80164B08 = D_80164B08 - 1;
        if (D_80164B08 != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
    block_status_fd:
        D_801609B8 = 0;
        D_801609A4 = 0xFD;
        goto block_return;
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
        goto block_return;
    c15_d70zero:
        phase_result = 5;
        D_801609A4 = 0xFC;
        D_80165488 = D_8016057C;
        goto block_return;

    cl_case_16:
        do
        {
            poll_result = func_80146104();
        } while (poll_result == -1);
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_17:
        D_80164A40 = 1;
        D_801609B8 = 0;
        func_8001729C(D_801609A8);
        D_8016548C = func_8001680C(D_801654E0, 0x8001);
        if (D_8016548C == -1)
        {
            goto block_return;
        }
        func_80145FC0();
        func_8001729C(D_801609A8);
        if (func_8001681C(D_8016548C, D_80165208,
                           D_80164A50 != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(D_8016548C);
            goto block_return;
        }
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_18:
        if (D_80164A40 == 0)
        {
            goto c18_increment;
        }
        poll_result = func_80146070();
        if (poll_result == 0)
        {
            D_80164A40 = 0;
            D_801609B8 = 1;
            func_8001683C(D_8016548C);
            goto block_return;
        }
        if (poll_result == -1)
        {
            goto block_return;
        }
        func_8001683C(D_8016548C);
        D_801609A4 = 0xFF;
        D_80165488 = D_80160574;
        goto block_return;
    c18_increment:
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_19:
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
                goto block_return;
            }
            goto block_return;
        }
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_20:
        poll_result20 = func_80146070();
        if (poll_result20 == 0)
        {
            D_80160934 = 0;
            D_80165488 = D_80165488 + 1;
            func_8001683C(D_8016548C);
            goto block_return;
        }
        if (poll_result20 < 0)
        {
            goto block_return;
        }
        if (poll_result20 >= 4)
        {
            goto block_return;
        }
        func_8001683C(D_8016548C);
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_read;
        }
        goto block_decrement_step;

    cl_case_24:
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
                goto block_return;
            }
        }
        func_80142B1C(3);
        goto block_return;

    cl_case_30:
        D_80164A48 = 5;
    block_case30_increment:
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_27:
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
                goto block_return;
            }
            goto block_return;
        }
        D_80165488 = D_80165488 + 1;
        goto block_return;

    cl_case_28:
        poll_result20 = func_80146070();
        if (poll_result20 == 0)
        {
            D_80160934 = 0;
            D_80165488 = D_80165488 + 1;
            func_8001683C(D_8016548C);
            goto block_return;
        }
        if (poll_result20 < 0)
        {
            goto block_return;
        }
        if (poll_result20 >= 4)
        {
            goto block_return;
        }
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_write_read;
        }
        goto block_close_decrement;

    cl_case_25:
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
            goto block_return;
        }
        goto block_return;

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
        goto block_return;

    cl_case_26:
        poll_result20 = func_80146070();
        if (poll_result20 != 0)
        {
            if (poll_result20 < 0)
            {
                goto block_return;
            }
            if (poll_result20 >= 4)
            {
                goto block_return;
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
        goto block_return;

    block_case26_retry:
        D_80164A48 = D_80164A48 - 1;
        if (D_80164A48 == 0)
        {
            D_80164A4C = 0;
            goto block_dialog_write;
        }
        goto block_close_decrement;

    }

block_close_decrement:
    func_8001683C(D_8016548C);
block_decrement_step:
    D_80165488 = D_80165488 - 1;

block_return:
    return phase_result;
}
