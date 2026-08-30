#include "common.h"

extern s32 D_8011F3AC;

void func_800A4A0C(void);
void func_800A4838(void);
void func_800A496C(void);
void func_800A4D1C(s32 arg0);

s32 func_800A4798(s32 arg0)
{
    s32 state;

    state = D_8011F3AC;
    if (state == 0)
    {
        return 0;
    }

    switch (state)
    {
        case 1:
            func_800A4A0C();
            break;

        case 2:
            func_800A4838();
            break;

        case 3:
            func_800A496C();
            break;
    }

    func_800A4D1C(arg0);
    return 1;
}
