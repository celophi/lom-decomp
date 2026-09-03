#include "common.h"
#include "sdk/libgpu.h"

extern s32 D_800CA898;
extern s32 D_800CA89C;

extern void func_80019C74(void*, s32);
extern void func_8002054C(s32);
extern void func_80019FB8(void*);
extern void func_800157DC(void);
extern void func_800196F0(s32);
extern void func_80019788(s32);
extern void func_800157B0(s32);
extern void func_8001990C(void*, s32, s32, s32);
extern void func_80019DEC(void*);
extern void func_80019D7C(void*);
extern void func_800122C0(void);
extern void func_800158E0(void);
extern void func_800500D8(void*);
extern void func_800503F0(void*);
extern void func_800517BC(void);

/**
 * @brief WSEL (world select) menu double-buffered present/update loop.
 * @param arg Base of the two-element render context (0x80CC-byte buffers).
 * @see field_run_frame_loop (analogous field-overlay loop)
 * @see (100%)
 */
void func_8004FD24(void* arg)
{
    void* cur;
    u_long* ot;
    RECT rect;

    cur = arg;
    func_80019C74((u8*)arg + 0x40, 0x1000);
    func_80019C74((u8*)arg + 0x810C, 0x1000);
    func_8002054C(0);
    func_80019FB8((u8*)arg + 0x4040);
    func_800157DC();
    func_800196F0(1);
    do
    {
        ot = (u_long*)((u8*)cur + 0x40);
        func_80019C74(ot, 0x1000);
        *(u32*)((u8*)cur + 0x80B8) = (u32)((u8*)cur + 0x40B8);
        func_8002054C(1);
        func_800500D8(cur);
        func_800503F0(cur);
        func_800517BC();
        func_80019788(0);
        func_800157B0(2);
        func_8002054C(2);
        func_8001990C((u8*)cur + 0x40B0, 0, 0, 0);
        if (cur == arg)
        {
            cur = (u8*)cur + 0x80CC;
            D_800CA898 = 1;
        }
        else
        {
            cur = arg;
            D_800CA898 = 0;
        }
        func_80019FB8((u8*)cur + 0x4040);
        func_80019DEC((u8*)cur + 0x4054);
        func_80019D7C(ot + 0xFFF);
        func_800157DC();
        func_800122C0();
    } while (D_800CA89C == 0);
    func_800158E0();
    func_8002054C(0);
}
