#include "common.h"

typedef struct {
    u16 field_0;
    u16 field_2;
} ZukanResourceEntry;

extern s32 D_80122988;
extern s32 D_80157528;
extern ZukanResourceEntry D_80157530[];
extern s16 D_80157D36;
extern s32 D_80157D3C;
extern s32 D_80157D40;
extern s32 D_80157D54;
extern s32 D_80157D58;
extern s32 D_80157D60;
extern s32 D_80157D64;
extern s32 D_80157D78;

s32 func_80141354()
{
    s32 count;
    s32 moved;
    s32 limit;
    s32 max;
    s32 forward;
    s32 backward;
    s32 value;

    if (D_80157D64 != 0)
        return;

    if (D_80122988 & 0x800) {
        D_80157528 = 1;
        return;
    }

    if (D_80157D58 != 0) {
        if (D_80157D36 != 0)
            return;

        if (D_80122988 & 0x40) {
            D_80157528 = 1;
            return;
        }

        moved = 0;
        if (D_80122988 & 0x220) {
            if (D_80157530[D_80157D60].field_2 >> 15) {
                func_800A3938(0x7E, 0x80);
                func_801424D0(0, 0, 0, 6);
                func_80142884(D_80157D60);
                D_80157D64 = 6;
                D_80157D54 = 0;
            } else {
                func_800A3938(0x78, 0x80);
            }
            return;
        }

        count = 1;
        if (D_80122988 & 8) {
            count = 8;
            D_80122988 = 0x4000;
        } else if (D_80122988 & 4) {
            count = 8;
            D_80122988 = 0x1000;
        }

        if (count != 0) {
            forward = D_80122988 & 0x6000;
            backward = D_80122988 & 0x9000;
            max = D_80157D3C;
            limit = max - 1;
            do {
                if (forward) {
                    D_80157D60++;
                    if (D_80157D60 >= max)
                        D_80157D60 = 0;
                    moved = 1;
                } else if (backward) {
                    D_80157D60--;
                    if (D_80157D60 < 0)
                        D_80157D60 = limit;
                    moved = 1;
                }
                if ((D_80157D60 == limit) || (D_80157D60 == 0))
                    count = 1;
                count--;
            } while (count != 0);
        }

        if (moved != 0) {
            func_800A3938(0x7D, 0x80);
            func_80141740();
        }
        return;
    }

    if (D_80122988 & 0x260) {
        func_800A3938(0x7E, 0x80);
        func_801424D0(0, 0, 0, 6);
        D_80157D64 = 5;
        D_80157D54 = 0;
        return;
    }

    if (D_80122988 & 0x2008) {
        func_800A3938(0x7D, 0x80);
        func_80141B90();
        value = D_80157D78;
        if ((value != 0) && (D_80122988 & 0x2000)) {
            func_801428C4(value);
            return;
        }

        D_80157D60++;
        if (D_80157D60 >= D_80157D3C)
            D_80157D60 = 0;
        while ((D_80157530[D_80157D60].field_2 >> 15) == 0) {
            D_80157D60++;
            if (D_80157D60 >= D_80157D3C)
                D_80157D60 = 0;
        }
        func_80142884(D_80157D60);
        func_80141740();
        return;
    }

    if (D_80122988 & 0x8004) {
        func_800A3938(0x7D, 0x80);
        func_80141BCC();
        value = D_80157D40;
        if ((value != 0) && (D_80122988 & 0x8000)) {
            func_801428C4(value);
            return;
        }

        D_80157D60--;
        if (D_80157D60 < 0)
            D_80157D60 = D_80157D3C - 1;
        while ((D_80157530[D_80157D60].field_2 >> 15) == 0) {
            D_80157D60--;
            if (D_80157D60 < 0)
                D_80157D60 = D_80157D3C - 1;
        }
        func_80142884(D_80157D60);
        func_80141740();
    }
}
