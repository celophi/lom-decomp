#include "common.h"

typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
} Struct_D80051ECC;

extern Struct_D80051ECC D_80051ECC;

void func_800AAFEC(Struct_D80051ECC *arg0);

void func_800C72A4(void)
{
    Struct_D80051ECC sp10;

    sp10 = D_80051ECC;
    func_800AAFEC(&sp10);
}
