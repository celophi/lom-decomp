#include "common.h"
#include "akao_cmd.h"

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern u8 D_800DCEF4[4];
extern WmapConfigA D_800D9268[];
extern s32 D_80139234;
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A79E4(void);

/** @brief Clear four actor states and start the next timed sequence step. */
void func_800A7940(void)
{
    s32 i;

    if (!(D_800DCEF4[3] & (D_800DCEF4[2] & (D_800DCEF4[0] & D_800DCEF4[1]))))
    {
        akao_cmd_f1();
    }
    for (i = 0; i < 4; i++)
    {
        D_800D9268[i + 20].field_0E = 0;
    }
    D_80139234 = -1;
    D_801B2E44 = 0x28;
    D_801B2E40++;
    func_800A79E4();
}
