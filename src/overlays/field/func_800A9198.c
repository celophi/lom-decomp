#include "common.h"

typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x54 - 0x26];
} StructFE054;

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x10 - 8];
    s32 unk10;
    u8 pad14[0x64 - 0x14];
    s32 unk64;
    u8 pad68[0x23C - 0x68];
} Struct106194;

typedef struct
{
    u8 pad0[0x2E];
    u8 unk2E;
    u8 pad2F[0x33 - 0x2F];
    u8 unk33;
    u8 pad34[0x48 - 0x34];
} RecFE3A0;

void func_800AA02C(void);
void akao_cmd_99_9b_9d_9f(s32 arg0);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void akao_stop_sfx_by_id(s32 id);

extern StructFE054 D_800FE054[];
extern RecFE3A0 D_800FE3A0[];
extern s32 D_800FE754;
extern Struct106194 D_80106194[];
extern s8 D_8011F3D2;
extern u8 D_801226E0[];
extern u8 D_801227D0;
extern u8 D_801228D0[];
extern u8 D_801228E0[];

/**
 * @brief Save actor sub-part state for live actors matching the current field group.
 */
void func_800A9198(void)
{
    StructFE054 *t0;
    Struct106194 *a0;
    s32 a2;
    u8 *base0;

    func_800AA02C();
    akao_cmd_99_9b_9d_9f(2);
    func_800A3904(0, 0x3C, 0);
    akao_stop_sfx_by_id(0x7E);
    t0 = D_800FE054;
    a2 = 3;
    base0 = (u8 *)D_80106194;
    a0 = (Struct106194 *)base0;
    D_801227D0 = 0;
    D_8011F3D2 = 0;
    do
    {
        if (t0->unk25 != 0xFF && a0->unk4 != 0 && D_800FE754 == (a0->unk10 & 0xF) && a0->unk64 != 0)
        {
            D_801226E0[D_801227D0] = a2;
            D_801228D0[D_801227D0] = D_800FE3A0[a2].unk2E;
            D_801228E0[D_801227D0] = D_800FE3A0[a2].unk33;
            D_801227D0 += 1;
        }
        a2 += 1;
        a0 = (Struct106194 *)((u8 *)a0 + 0x23C);
        t0 = (StructFE054 *)((u8 *)t0 + 0x54);
    } while (a2 < 0xD);
}
