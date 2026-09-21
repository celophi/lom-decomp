#include "common.h"

extern void cdrom_wait_queue_empty(void);
extern void func_800651B4(void *);
extern void func_800652A8(s32, s32);
extern void func_8006CAC0(void (*callback)(void));
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 D_801B2E80;
extern s32 D_801B2E84;
extern void func_800AD23C(void);

/** @brief Wait for queued CD work, initialize three buffers, and register the next callback. */
void func_800ABA8C(void)
{
    cdrom_wait_queue_empty();
    func_800652A8(0x2D, 0x80);
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_800651B4(&D_80193640);
    func_8006CAC0(&func_800AD23C);
    D_801B2E84 = 0x12;
    D_801B2E80 += 1;
}
