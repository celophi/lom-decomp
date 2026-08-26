#include "common.h"

typedef struct
{
    s8 pad[0xAA9];
    u8 unkAA9;
} UnkStruct80122B74;

extern UnkStruct80122B74 *D_80122B74;

void func_800C3B50(void);

s32 func_800C318C(void)
{
    func_800C3B50();
    return D_80122B74->unkAA9 + 0x41;
}
