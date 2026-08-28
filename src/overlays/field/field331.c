#include "common.h"

extern s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

typedef struct {
    u8 unk0;   /* 0x0 */
    u8 unk1;   /* 0x1 */
} StructEC;

typedef struct {
    u8 pad0[0x3C];
    u8 unk3C;   /* 0x3C */
    u8 pad3D[0x40B8 - 0x3D];
    s32 unk40B8; /* 0x40B8 */
} ArgA;

extern StructEC D_800EC3D2;
extern StructEC D_800EC3D4;

/**
 * @note NOT YET MATCHED (92.88%). target frame is 0x38 with the call setup duplicated per if/else arm and 2 extra spills; this C shares more and comes out 0x30. The two-call branch structure diverges from the single-call queued-draw twins, so the shared recipe does not stamp it.
 * @see decomp.me (92.88%) TODO
 */
void func_800A92CC(ArgA *arg0)
{
    s32 handle;
    void *p3C;

    handle = arg0->unk40B8;
    p3C = &arg0->unk3C;
    if (cdrom_get_error_status() == 2)
    {
        handle = func_800A88A0(
            handle, p3C,
            D_800EC3D2.unk0 + ((D_800EC3D2.unk1 << 8) + ((u8 *)&D_800EC3D2 - 0xE)),
            4, 0xA0, 0x64, 0x82);
    }
    else
    {
        handle = func_800A88A0(
            handle, p3C,
            D_800EC3D4.unk0 + ((D_800EC3D4.unk1 << 8) + ((u8 *)&D_800EC3D4 - 0x10)),
            4, 0xA0, 0x64, 0x82);
    }
    arg0->unk40B8 = handle;
}
