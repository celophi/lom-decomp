#include "common.h"

typedef struct {
    s32 unk0;
    s16 unk4;
    u8 pad[0x62];
} NikiLoadScratch;

typedef struct {
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} NikiFileHeaderScratch;

typedef struct {
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} NikiFileHeader;

extern NikiFileHeader D_80140090;
extern void *jtbl_80140098[];
extern char D_800ECF9C[];
extern char D_800ECFB0[];
extern s32 D_80164B78;
extern s32 D_80164B70;
extern s32 D_80164B7C;
extern s32 D_80164B84;
extern s32 D_80164AD4;
extern s32 D_80164AE8;
extern u8 *D_80164E18;
extern s32 D_80164E1C;
extern s32 D_80164E20[];
extern s32 D_80164EB0;
extern s32 D_80164A78;
extern s32 D_80164F08;
extern s32 D_80164B94;
extern s32 D_80164EB4;
extern s32 D_80164F10;
extern s32 D_80164F0C;
extern s32 D_80164F18;
extern s32 D_80164FC0;
extern s32 D_80164FD4;
extern u8 D_80164FD8[];
extern u8 D_80165018[];
extern s32 D_80164B90;
extern u8 D_80164B98[];
extern u8 D_80164E70[];
extern u8 D_80160A78[];
extern u8 D_801606C8[];
extern u8 D_801606D0[];

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
s32 func_80145CBC(s32);
s32 func_80145DA4(s32);
void func_80145F68(void);
void func_80146114(void);
void func_8014616C(void);
s32 func_801461C4(void);
s32 func_80146258(void);
void func_80142B2C(s32);
void func_80142C18(s32);

static inline void niki_probe_render_two(void)
{
    NikiFileHeaderScratch p;

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_80164B70;
    func_80016F9C(&p, &D_800ECF9C);
    func_8001686C(&p);

    memcpy(&p, &D_80140090, 6);
    ((u8 *)&p)[2] += *(u8 *)&D_80164B70;
    func_80016F9C(&p, &D_800ECFB0);
    func_8001686C(&p);
}

s32 func_80144DF8(void)
{
    NikiLoadScratch buf;
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
    phase_result = 1;
    ((u8 *)&buf)[2] += *(u8 *)&D_80164B70;

    if (D_80164E18 == NULL)
    {
        goto block_return;
    }

    switch (0)
    {
    case 0:
        dispatch = *D_80164E18;
        if ((u32)dispatch >= 0x1F)
        {
            goto block_return;
        }
        goto *jtbl_80140098[dispatch];

    cl_case_1:
        phase_result = 3;
        func_8001729C(D_80164B70);
        func_8001724C(D_80164B70 * 0x10);
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_2:
        poll_result = func_801461C4();
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
            goto block_increment;
        }
        goto block_return;
    c2_ge3:
        if (poll_result == 3)
        {
            goto c2_eq3;
        }
        goto block_return;
    c2_pos:
        phase_result = 4;
        D_80164B84 = 0;
        D_80164B78 = 0xFD;
        D_80164E18 = D_80164E18 + 1;
        goto block_return;
    c2_eq3:
        D_80164EB0 = 0x28;
        rank_value = -1;
        for (rank_index = 14; rank_index >= 0; rank_index--)
        {
            D_80164E20[rank_index] = rank_value;
        }
        goto block_status_ff;

    cl_case_3:
        func_80146114();
        goto block_increment;

    cl_case_4:
        do
        {
            poll_result = func_80146258();
        } while (poll_result == -1);
        if (poll_result == 0)
        {
            goto block_increment;
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
        func_8014616C();
        goto block_increment;

    cl_case_6:
        niki_probe_render_two();
        D_80164F18 = 1;
        if (func_80145CBC(D_80164B70) == 0)
        {
            phase_result = 2;
            D_80164E18 = NULL;
            D_80164B78 = 0xF8;
            D_80164F18 = 0;
            goto block_return;
        }
        wait_attempts = 0;
        D_80164E18 = D_80164E18 + 1;
        do
        {
            if (func_80145DA4(D_80164B70) == 0)
            {
                if (D_80164AE8 != 0)
                {
                    D_80164B7C = 0;
                }
                D_80164F18 = 0;
                if (D_80164B78 == 0xF8)
                {
                    goto block_return;
                }
                if (D_80164B78 == 0xFA)
                {
                    goto block_return;
                }
                func_80145F68();
                goto block_return;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        goto block_return;

    cl_case_8:
        phase_result = 3;
        func_8001729C(D_80164B70);
        func_800172AC(D_80164B70 * 0x10);
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_9:
        phase_result = 3;
        func_8001729C(D_80164B70);
        func_8001725C(D_80164B70 * 0x10);
        D_80164F0C = 0x10;
        D_80164FC0 = 0x10;
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_0:
        phase_result = 2;
        D_80164B90 = 0;
        goto block_return;

    cl_case_10:
        func_80016F9C(&buf, D_80165018 + (D_80164B70 * 0x320) + (D_80164B7C * 0x28));
        wait_attempts = 0;
        func_8001729C(D_80164B70);
        do
        {
            poll_result = func_8001686C(&buf);
            wait_attempts = wait_attempts + 1;
            if (poll_result != 0)
            {
                break;
            }
        } while (wait_attempts < 0x14);
        goto block_increment;

    cl_case_15:
        poll_result = func_801461C4();
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
            goto block_increment;
        }
        goto block_return;
    c15_ge3:
        if (poll_result == 3)
        {
            goto c15_eq3;
        }
        goto block_return;
    c15_pos:
        D_80164FC0 = D_80164FC0 - 1;
        if (D_80164FC0 != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
    block_status_fd:
        D_80164B84 = 0;
        D_80164B78 = 0xFD;
        goto block_return;
    c15_eq3:
        D_80164F0C = D_80164F0C - 1;
        if (D_80164F0C == 0)
        {
            goto c15_d70zero;
        }
    block_reissue:
        func_8001729C(D_80164B70);
        func_800172AC(D_80164B70 * 0x10);
        func_8001729C(D_80164B70);
        func_8001725C(D_80164B70 * 0x10);
        goto block_return;
    c15_d70zero:
        phase_result = 5;
        D_80164B78 = 0xFC;
        D_80164E18 = D_801606D0;
        goto block_return;

    cl_case_16:
        do
        {
            poll_result = func_80146258();
        } while (poll_result == -1);
        goto block_increment;

    cl_case_17:
        D_80164A78 = 1;
        D_80164B84 = 0;
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164E70, 0x8001);
        if (D_80164E1C == -1)
        {
            goto block_return;
        }
        func_80146114();
        func_8001729C(D_80164B70);
        if (func_8001681C(D_80164E1C, D_80164B98,
                           D_80164EB4 != 0 ? 0x280 : 0x80) == -1)
        {
            func_8001683C(D_80164E1C);
            goto block_return;
        }
        goto block_increment;

    cl_case_18:
        poll_result = func_801461C4();
        if (poll_result == 0)
        {
            D_80164A78 = 0;
            D_80164B84 = 1;
            D_80164E18 = D_80164E18 + 1;
            func_8001683C(D_80164E1C);
            goto block_return;
        }
        if (poll_result == -1)
        {
            goto block_return;
        }
        D_80164A78 = 0;
        func_8001683C(D_80164E1C);
    block_status_ff:
        D_80164B78 = 0xFF;
        D_80164E18 = D_801606C8;
        goto block_return;

    cl_case_19:
        D_80164AD4 = 1;
        D_80164F08 = 1;
        D_80164F10 = func_8002054C(-1);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164E70, 0x8001);
        func_80146114();
        func_8001729C(D_80164B70);
        if (func_8001681C(D_80164E1C, D_80160A78, 0x4000) == -1)
        {
            D_80164B94 = D_80164B94 - 1;
            if (D_80164B94 == 0)
            {
            block_dialog_read:
                func_80142B2C(1);
                goto block_return;
            }
            goto block_return;
        }
        goto block_increment;

    cl_case_20:
        poll_result20 = func_801461C4();
        if (poll_result20 == 0)
        {
            D_80164AD4 = 0;
            D_80164E18 = D_80164E18 + 1;
            func_8001683C(D_80164E1C);
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
        D_80164B94 = D_80164B94 - 1;
        if (D_80164B94 == 0)
        {
            D_80164F08 = 0;
            goto block_dialog_read;
        }
        goto block_decrement_step;

    cl_case_24:
        wait_attempts = 0;
        do
        {
            if (func_800342CC(D_80164B70 * 0x10) == 1)
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
                goto block_increment;
            }
        }
        func_80142B2C(3);
        goto block_return;

    cl_case_27:
        D_80164AD4 = 1;
        D_80164F08 = 1;
        D_80164F10 = func_8002054C(-1);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164E70, 0x8001);
        func_80146114();
        func_8001729C(D_80164B70);
        if (func_8001681C(D_80164E1C, D_80160A78, 0x4000) == -1)
        {
            func_8001683C(D_80164E1C);
            D_80164B94 = D_80164B94 - 1;
            if (D_80164B94 == 0)
            {
            block_dialog_write_read:
                func_80142C18(1);
                goto block_return;
            }
            goto block_return;
        }
        goto block_increment;

    cl_case_28:
        poll_result20 = func_801461C4();
        if (poll_result20 == 0)
        {
            D_80164AD4 = 0;
            D_80164E18 = D_80164E18 + 1;
            func_8001683C(D_80164E1C);
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
        D_80164B94 = D_80164B94 - 1;
        if (D_80164B94 == 0)
        {
            func_8001683C(D_80164E1C);
            D_80164F08 = 0;
            func_80142C18(1);
            return phase_result;
        }
        goto block_close_decrement;

    cl_case_30:
        D_80164B94 = 5;
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_25:
        if (D_80164FD4 == 0)
        {
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164E70) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&buf, D_800ECF9C);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(&buf, 0x20200);
        if (D_80164E1C != -1)
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
        D_80164B94 = D_80164B94 - 1;
        if (D_80164B94 == 0)
        {
        block_dialog_write:
            func_80142C18(0);
            goto block_return;
        }
        goto block_return;

    block_write_opened:
        func_8001683C(D_80164E1C);
        func_800170BC(D_80164FD8, &buf);
        func_8001729C(D_80164B70);
        D_80164E1C = func_8001680C(D_80164FD8, 0x8002);
        func_80146114();
        D_80164F08 = 1;
        D_80164F10 = func_8002054C(-1);
        func_8001729C(D_80164B70);
        if (func_8001682C(D_80164E1C, D_80160A78, 0x4000) == -1)
        {
            func_8001683C(D_80164E1C);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164FD8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            goto block_write_retry;
        }
        goto block_increment;

    block_increment:
        D_80164E18 = D_80164E18 + 1;
        goto block_return;

    cl_case_26:
        poll_result20 = func_801461C4();
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
        if (D_80164FD4 != 0)
        {
            func_8001729C(D_80164B70);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(D_80164E70) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(D_80164B70);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(D_80164FD8, D_80164E70) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80164B90 = 0;
        D_80164E18 = D_80164E18 + 1;
        func_8001683C(D_80164E1C);
        goto block_return;

    }

block_case26_retry:
    D_80164B94 = D_80164B94 - 1;
    if (D_80164B94 == 0)
    {
        goto block_case26_exhausted;
    }

block_close_decrement:
    func_8001683C(D_80164E1C);
block_decrement_step:
    D_80164E18 = D_80164E18 - 1;
    goto block_return;

block_case26_exhausted:
    D_80164F08 = 0;
    func_80142C18(0);
    wait_attempts = 0;
    do
    {
        if (func_8001686C(D_80164FD8) != 0)
        {
            break;
        }
        wait_attempts = wait_attempts + 1;
    } while (wait_attempts < 0x14);

block_return:
    return phase_result;
}
