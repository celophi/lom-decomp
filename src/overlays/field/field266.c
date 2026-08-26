#include "common.h"

typedef struct {
    u8 pad[0x28];
    s32 unk28;
    s32 unk2C;
    s32 unk30;
} SomeStruct;

extern void func_8008BD88(s32 arg0);
extern SomeStruct *func_800C1B60(s32 arg0);

void func_800C1D14(s32 arg0, s32 arg1) {
    s32 temp_a1;
    SomeStruct *temp_v0;

    temp_a1 = arg1 & 1;
    if (temp_a1 != 0) {
        func_8008BD88(arg0);
    }
    temp_v0 = func_800C1B60(arg0);
    temp_v0->unk2C = 0;
    temp_v0->unk30 = 0;
    temp_v0->unk28 = temp_v0->unk28 & 0x7FFFFFFF;
}
