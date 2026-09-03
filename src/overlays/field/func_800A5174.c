#include "common.h"

typedef struct { s32 unk0; s32 unk4; s32 unk8; } BufHdr;
extern u8 *D_8010D038;
extern u8 D_8010A038[];
extern u8 D_801148B0[];
void cdrom_queue_read(s32 id, void *dest);
void cdrom_wait_queue_empty(void);
void bcopy(const void *src, void *dest, s32 size);

void func_800A5174(s32 arg0, s32 arg1)
{
    s32 *temp_v0;
    s32 temp_v0_2;
    BufHdr *temp_s0;
    u8 *temp_a0;
    s32 count;

    temp_s0 = (BufHdr *)D_8010D038;
    cdrom_queue_read(arg1 & 0xFFFF, temp_s0);
    cdrom_wait_queue_empty();
    temp_v0 = (s32 *)(D_8010D038 + temp_s0->unk0);
    temp_a0 = (u8 *)temp_v0 + 4;
    count = *temp_v0;
    if (arg0 == 2) {
        u8 *dst = D_8010A038;
        dst += 0x320;
        bcopy(temp_a0, dst, count * 8);
    }
    temp_s0 = (BufHdr *)((u8 *)temp_s0 + 4);
    {
        u8 *src = D_8010D038;
        s32 end;
        temp_v0_2 = temp_s0->unk0;
        end = temp_s0->unk4;
        src += temp_v0_2;
        bcopy(src, D_801148B0 + (arg0 << 12), end - temp_v0_2);
    }
}
