#include "common.h"
#include "vector.h"

extern s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

typedef struct
{
    u8 pad0[0x3C];
    u8 unk3C;
    u8 pad3D[0x40B8 - 0x3D];
    s32 unk40B8;
} ArgA;

extern StructEC D_800EC3D2;
extern StructEC D_800EC3D4;

/**
 * @brief Draw the field status text selected by the CD-ROM error state.
 * @param arg0 Render context containing the text ordering table and primitive cursor.
 */
void func_800A92CC(ArgA *arg0)
{
    s32 handle;
    void *ordering_table;
    Vec2s text_positions[2];

    handle = arg0->unk40B8;
    ordering_table = &arg0->unk3C;
    if (cdrom_get_error_status() == 2)
    {
        s32 low;
        s32 offset;
        u8 *base;

        low = D_800EC3D2.unk0;
        offset = (D_800EC3D2.unk1 << 8) + (s32)(base = (u8 *)&D_800EC3D2 - 0xE);
        handle = func_800A88A0(handle, ordering_table, (void *)(low + offset), 4, 0xA0, 0x64, 0x82);
    }
    else
    {
        s32 low;
        s32 offset;
        u8 *base;

        low = D_800EC3D4.unk0;
        offset = (D_800EC3D4.unk1 << 8) + (s32)(base = (u8 *)&D_800EC3D4 - 0x10);
        handle = func_800A88A0(handle, ordering_table, (void *)(low + offset), 4, 0xA0, 0x64, 0x82);
    }
    arg0->unk40B8 = handle;
}
