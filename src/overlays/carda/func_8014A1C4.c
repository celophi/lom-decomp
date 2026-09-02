#include "common.h"

#define NENT 20

typedef struct
{
    u8 data[0x28];
} CardaEntry28;

extern s32 D_801660A0;
extern s32 D_80165FEC;
extern CardaEntry28 D_80166440[][NENT];
extern s32 D_80166A80[];
extern char D_800ECF7C[];
extern char D_800ECF8C[];
extern char D_800ECFC4[];
extern char D_800ECFD0[];
extern s32 func_8001714C();
extern void func_80016E7C();

void func_8014A1C4(void)
{
    CardaEntry28 sorted[NENT];
    s32 out;
    s32 group;
    s32 i;

    out = 0;
    group = 0;
    do
    {
        do
        {
            i = 0;
        } while (0);
        if (D_80165FEC > 0)
        {
            do
            {
                if (D_80166A80[i] == group &&
                    func_8001714C(D_800ECF7C, &D_80166440[D_801660A0][i], 0xC) == 0)
                {
                    func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80165FEC);
        }
        group++;
    } while (group < 8);

    group = 0;
    do
    {
        do
        {
            i = 0;
        } while (0);
        if (D_80165FEC > 0)
        {
            do
            {
                if (D_80166A80[i] == group &&
                    func_8001714C(D_800ECF8C, &D_80166440[D_801660A0][i], 0xC) == 0)
                {
                    func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80165FEC);
        }
        group++;
    } while (group < 8);

    do
    {
        i = 0;
    } while (0);
    if (D_80165FEC > 0)
    {
        do
        {
            if (func_8001714C(D_800ECFC4, &D_80166440[D_801660A0][i], 8) == 0 ||
                func_8001714C(D_800ECFD0, &D_80166440[D_801660A0][i], 9) == 0)
            {
                func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80165FEC);
    }

    if (*(volatile s32 *)&D_80165FEC > 0)
    {
        do
        {
            i = 0;
        } while (0);
        do
        {
            if (func_8001714C(D_800ECF7C, &D_80166440[D_801660A0][i], 0xC) != 0 &&
                func_8001714C(D_800ECF8C, &D_80166440[D_801660A0][i], 0xC) != 0 &&
                func_8001714C(D_800ECFC4, &D_80166440[D_801660A0][i], 8) != 0 &&
                func_8001714C(D_800ECFD0, &D_80166440[D_801660A0][i], 9) != 0)
            {
                func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80165FEC);
    }

    do
    {
        i = 0;
    } while (0);
    if (D_80165FEC > 0)
    {
        do
        {
            func_80016E7C(&sorted[i], &D_80166440[D_801660A0][i], 0x28);
            i++;
        } while (i < D_80165FEC);
    }
}
