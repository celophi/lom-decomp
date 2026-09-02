#include "common.h"
#define NENT 20
typedef struct { u8 data[0x28]; } NikiEntry28;
extern s32 D_80164B70;
extern s32 D_80164B78;
extern NikiEntry28 D_80165018[][NENT];
extern s32 D_80164EB8[];
extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern s32 func_8001714C();
extern void func_80016E7C();

void func_801462EC(void)
{
    NikiEntry28 sorted[NENT];
    s32 out = 0;
    s32 group = 0;
    s32 i;
    do {
        do { do { do { do { do { do { do { do { do { i = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
        if (D_80164B78 > 0) {
            do {
                if (D_80164EB8[i] == group &&
                    func_8001714C(D_800ECF7C, &D_80165018[D_80164B70][i], 0xC) == 0) {
                    func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80164B78);
        }
        group++;
    } while (group < 8);

    group = 0;
    do {
        do { do { do { do { do { do { do { do { do { i = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
        if (D_80164B78 > 0) {
            do {
                if (D_80164EB8[i] == group &&
                    func_8001714C(D_800ECF8C, &D_80165018[D_80164B70][i], 0xC) == 0) {
                    func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80164B78);
        }
        group++;
    } while (group < 8);

    do { do { do { do { do { do { do { do { do { i = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    if (D_80164B78 > 0) {
        do {
            if (func_8001714C(D_800ECFC4, &D_80165018[D_80164B70][i], 8) == 0) {
                func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80164B78);
    }

    if (*(volatile s32 *)&D_80164B78 > 0) {
        do { do { do { do { do { do { do { do { do { i = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
        do {
            if (func_8001714C(D_800ECF7C, &D_80165018[D_80164B70][i], 0xC) != 0 &&
                func_8001714C(D_800ECF8C, &D_80165018[D_80164B70][i], 0xC) != 0 &&
                func_8001714C(D_800ECFC4, &D_80165018[D_80164B70][i], 8) != 0) {
                func_80016E7C(&D_80165018[D_80164B70][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80164B78);
    }

    do { do { do { do { do { do { do { do { do { i = 0; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    if (D_80164B78 > 0) {
        do {
            func_80016E7C(&sorted[i], &D_80165018[D_80164B70][i], 0x28);
            i++;
        } while (i < D_80164B78);
    }
}
