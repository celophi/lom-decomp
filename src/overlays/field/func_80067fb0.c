#include "common.h"

void func_800A710C(void);
void func_8006441C(void);
void func_800A8880(s32);
void func_80063194(void);

extern s32 D_800F229C[];

/**
 * @see decomp.me (100%) TODO
 */
void func_80067FB0(s32 arg0)
{
    if (D_800F229C[0] != 0)
    {
        func_800A710C();
        if (D_800F229C[0] != 0)
        {
            func_8006441C();
            if (D_800F229C[0] != 0)
            {
                func_800A8880(arg0);
            }
            func_80063194();
        }
    }
}

typedef struct
{
    s16 r;                  // 0x00
    s16 g;                  // 0x02
    s16 b;                  // 0x04
    s16 steps;              // 0x06
} FieldFade;

void func_800AA02C(void);
void func_800A9A5C(void);
void func_800A68B4(void);
void func_800A7434(void);
void func_800A74B8(void);

extern FieldFade g_field_fade_target;
extern s32 D_8010D020[];

/**
 * @see decomp.me (100%) TODO
 */
void func_80068028(void)
{
    func_800AA02C();
    g_field_fade_target.r = 0xC0;
    g_field_fade_target.g = 0xC0;
    g_field_fade_target.b = 0xC0;
    g_field_fade_target.steps = 5;
    if (D_8010D020[0] == 0)
    {
        func_800A9A5C();
        func_800A68B4();
        func_800A7434();
    }
    else
    {
        func_800A74B8();
    }
}
