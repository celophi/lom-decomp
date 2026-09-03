#include "common.h"
#include "sdk/libgpu.h"

typedef struct {
    u8 _pad0[4];
    u16 x1;
    u16 y1;
    u16 x2;
    u16 y2;
    u8 _pad1[0x18 - 0xC];
} WselTexEntry;

extern WselTexEntry D_800C6720[];
extern int func_80019A34(RECT*, u_long*);

/**
 * @brief Load a WSEL texture (and, when the flag bit is set, a preceding CLUT
 *        block) into VRAM, using the destination RECT from entry table
 *        D_800C6720[index].
 * @param res   Resource header; its first sub-block is inline at +8.
 * @param index Slot into the 0x18-byte D_800C6720 entry table.
 * @note NON-MATCHING (~78%). Structure is byte-exact (55/55 insns, frame -0x30,
 *       all stack slots match); residue is a coupled register-allocation
 *       (res/size s0<->s1 swap) plus sched1 emit-order difference (D_800C6720
 *       base vs index computation order; size-read/pointer-advance placement).
 *       Register-pinning reaches ~80% but is not admissible in source;
 *       sched_oracle classifies the remainder as not a clean emit-order fix.
 *       TODO: recover the exact codegen (likely a different natural decomposition).
 * @see (78.42%)
 */
void func_800521D0(u8* res, s32 index)
{
    RECT rect;
    WselTexEntry* entry = &D_800C6720[index];
    s16 x1 = entry->x1;
    s16 y1 = entry->y1;
    s16 x2 = entry->x2;
    s16 y2 = entry->y2;

    if (res[4] & 8)
    {
        u_long* pix;
        u32 size;
        rect.w = *(u16*)(res + 0x10) * *(u16*)(res + 0x12);
        rect.x = x2;
        rect.y = y2;
        rect.h = 1;
        pix = (u_long*)(res + 0x14);
        size = *(u32*)(res + 8);
        res += 8;
        func_80019A34(&rect, pix);
        res += size;
    }
    else
    {
        res += 8;
    }
    rect.x = x1;
    rect.y = y1;
    rect.w = *(u16*)(res + 8);
    rect.h = *(u16*)(res + 0xA);
    func_80019A34(&rect, (u_long*)(res + 0xC));
}
