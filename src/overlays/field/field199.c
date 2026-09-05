#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EF8;

extern UnkStruct80051EF8 D_80051EF8;
extern void field_open_gosub_screen_sequence(UnkStruct80051EF8 *arg0);

void func_800C9894(void)
{
    UnkStruct80051EF8 local;

    local = D_80051EF8;
    field_open_gosub_screen_sequence(&local);
}
