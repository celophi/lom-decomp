#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EC0;

extern UnkStruct80051EC0 D_80051EC0;
extern void func_800AAFEC(UnkStruct80051EC0 *arg0);

/**
 * @brief Copy the D_80051EC0 constant onto the stack and forward it to func_800AAFEC.
 */
void func_800C7238(void)
{
    UnkStruct80051EC0 local;

    local = D_80051EC0;
    func_800AAFEC(&local);
}
