#include "common.h"

typedef struct CardaDirEntry {
    char name[20];
    s32 attr;
    s32 size;
    void *next;
    s32 head;
    char system[4];
} CardaDirEntry;

extern s32 D_80165FEC;
extern s32 D_80165FF4;
extern s32 D_80166078;
extern s32 D_801660A0;
extern CardaDirEntry D_80166440[][20];
extern s32 D_80166AD8;
extern s32 D_80166B88;
extern char D_800ECFC4[];
extern char D_800ECFD0[];

s32 func_8001684C();
void func_800170BC();
void func_8001729C();
void func_800AA02C();
void func_800B0170();
void func_801411CC();
s32 func_801477CC();
s32 func_80147C94();

s32 func_80149A4C(s32 page)
{
    s32 scan_i;
    s32 i;
    s32 sum;
    s32 selected;
    s32 count;
    s32 offset;
    s32 cond;
    CardaDirEntry *entries;

    func_8001729C(page);
    scan_i = 0;
    do {
        if (func_8001684C(&D_80166440[page][D_80165FEC]) != 0) {
            func_800B0170(&D_80166440[page][D_80165FEC]);
            D_80165FEC += 1;
            return 1;
        }
        scan_i++;
    } while (scan_i < 20);

    func_800AA02C();
    if (D_80166078 == 1 && func_80147C94() == 0) {
        D_80165FEC = 0xF8;
    } else {
        i = 0;
        sum = 0;
        D_80166B88 = 0;
        count = D_80165FEC;
        if (count > 0) {
            do { entries = (CardaDirEntry *)D_80166440; } while (0);
            offset = D_801660A0 * 0x320;
            do {
                do {
                    sum += ((CardaDirEntry *)((u32)offset + (u32)entries))->size / 8192;
                } while (0);
                i++;
                offset += 0x28;
            } while (i < count);
        }
        if (sum >= 14 || D_80166078 == 2) {
            if (sum < 10) {
                if (D_80166078 != 2)
                    goto cond_one;
                goto cond_zero;
            }
            goto cond_one;
        }
cond_zero:
        cond = 0;
        goto cond_done;
cond_one:
        cond = 1;
cond_done:
        if (cond != 0) {
            if (D_80166078 == 0 || D_80166078 == 2) {
                func_800170BC(&D_80166440[page][D_80165FEC], D_800ECFD0);
                D_80166440[page][D_80165FEC].size = 0;
                D_80165FEC += 1;
            }
            selected = func_801477CC();
            if (func_80147C94() == 0) {
                if ((u32)(D_80166078 - 2) < 2U) {
                    D_80165FEC = 0xF7;
                } else {
                    D_80165FEC = 0xFA;
                }
                D_80166AD8 = 0;
            } else {
                D_80165FF4 = selected;
                func_801411CC();
            }
        } else {
            D_80166B88 = 1;
            if ((u32)(D_80166078 - 2) < 2U) {
                func_800170BC(&D_80166440[page][D_80165FEC], D_800ECFC4);
                D_80166440[page][D_80165FEC].size = 0xC000;
                D_80165FEC += 1;
            } else if (D_80166078 == 0) {
                func_800170BC(&D_80166440[page][D_80165FEC], D_800ECFC4);
                D_80166440[page][D_80165FEC].size = 0x4000;
                D_80165FEC += 1;
            }
            selected = func_801477CC();
            if (func_80147C94() == 0) {
                D_80165FF4 = 0;
                func_801411CC();
                D_80166AD8 = 0;
            } else {
                D_80165FF4 = selected;
                func_801411CC();
            }
        }
    }
    return 0;
}
