#include "common.h"

typedef struct
{
    u32 flags;    /* 0x00 */
    u32 state;    /* 0x04 */
    u8 pad8[8];   /* 0x08 */
    void (*unk10)(void); /* 0x10 */
} FieldADF84Rec;

extern void func_800A3938(s32 sound_id, s32 pan);
extern void func_800ADF34(void);
extern FieldADF84Rec *func_800ADF84(void);
extern void func_800A7FB4(void);
extern s32 D_800F229C;
extern s32 D_80122908;

/**
 * @note NOT YET MATCHED (75.63%, insn count exact). Residue is SCHED-LUID:
 * the target hoists the 0xFFFFFF mask for the second rec->flags store all
 * the way to the top of the block and interleaves the first rec->flags
 * store with the D_80122908 load; our compile keeps both close to their
 * point of use instead. The do/while(0) wrappers below are load-bearing:
 * splitting the `state` build into two statements and fencing it (and the
 * byte-2/final-state stores) with a scheduler region boundary is what
 * recovers the `andi rX, rX, 0x78` mask the target keeps on the priority
 * byte computation - removing any of the three wrappers, or re-merging the
 * `state` assignment into one expression, measurably loses exact rows
 * (confirmed via probe_variants). Reordering statements or renaming/
 * splitting `rec->flags` into a local temp was inert.
 * @see decomp.me (75.63%) TODO
 */
void func_800A764C(void)
{
    FieldADF84Rec *rec;
    u32 state;

    D_800F229C = 2;
    func_800A3938(0xB9, 0x80);
    func_800ADF34();
    rec = func_800ADF84();
    rec->flags = ((rec->flags & ~0x78) | 8) & 0xFFFF007F | 0x2000;
    do
    {
        state = rec->state & ~0x1FE;
        state |= (((D_80122908 << 4) + 0x10) & 0xFF) << 1;
    } while (0);
    do
    {
        *((s8 *)rec + 2) = 0x70 - ((state >> 2) & 0x78);
    } while (0);
    rec->unk10 = func_800A7FB4;
    rec->state = state;
    rec->flags = (rec->flags & 0xFFFFFF) | 0xC0000000;
    do
    {
        rec->state = rec->state & ~1;
    } while (0);
}
