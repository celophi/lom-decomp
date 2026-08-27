#include "common.h"

typedef struct
{
    u8 pad0[0x50];
    s32 unk50;
    u8 pad54[4];
    s32 unk58;
} Obj80087F0C;

extern Obj80087F0C *func_80087F0C(s32 arg0);
extern s32 func_80087F44(s32 arg0, s32 *out);

s32 func_800C1FFC(s32 arg0, s32 arg1, s32 arg2)
{
    Obj80087F0C *obj;
    s32 buf[4];
    s32 bx;
    s32 by;

    obj = func_80087F0C(arg0);
    func_80087F44(arg0, buf);
    bx = buf[0];
    if ((bx - arg1) < obj->unk50 && obj->unk50 < (bx + arg1))
    {
        by = buf[2];
        if ((by - arg2) < obj->unk58 && obj->unk58 < (by + arg2))
        {
            return -1;
        }
    }
    return 0;
}
