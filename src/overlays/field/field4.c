#include "common.h"


/* Define the structure layout for the memory at 0x801ED600 */
typedef struct {
    u8 pad0[0x91];
    u8 unk91;
    u8 unk92;
    u8 pad93[0xAC]; /* 0x13F - 0x093 = 0xAC bytes of padding */
    u8 unk13F;
    u8 unk140;
} UnkStruct_801ED600;

s32 cdrom_stream(s32 resourceIndex, u32 destination);
void cdrom_wait_queue_empty(void);
extern void func_80084240(void);                                 /* Fixed prototype */
void func_80140004(s32 cdLoadAddr, s32 imageResourceIndex, s32 musicResourceIndex, s32 audioClipIndex);

extern s32 D_800F22B8;
extern s32 D_800F22BC;
extern s32 D_800F22C0;
extern s32 D_800F22C4;

void func_8006828C(void)
{
    /* Declaring the pointer here forces the early lui/ori instructions for v1 */
    UnkStruct_801ED600* ptr = (UnkStruct_801ED600*)0x801ED600;
    s32 temp_v0;

    if (D_800F22C0 != 0)
    {
        temp_v0 = D_800F22C0 - 1;
        D_800F22C0 = temp_v0;
        if (temp_v0 == 0)
        {
            ptr->unk140 = 0;
            ptr->unk92 = 0;
            ptr->unk13F = 0;
            ptr->unk91 = 0;
            cdrom_stream(0xA, 0x80140000);
            cdrom_wait_queue_empty();
            func_80140004(0x80160000, D_800F22B8, D_800F22BC, D_800F22C4);
            func_80084240();
        }
    }
}