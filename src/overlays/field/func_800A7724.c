#include "common.h"

/** @brief Process flags, packed state, and update callback. */
typedef struct
{
    union
    {
        u32 flags;
        struct
        {
            u8 byte0;
            u8 byte1;
            u8 position;
            u8 byte3;
        } bytes;
    } attributes;
    u32 state;
    u8 pad8[0x10 - 8];
    void (*update)(void);
} FieldProcEntry;

/** @brief Actor slot prefix containing the active flag. */
typedef struct
{
    u8 unk0;
    u8 pad1[0x268 - 1];
} Entry268;

extern FieldProcEntry *func_800ADF84(void);
extern Entry268 D_800FD818[];
extern s32 D_800F229C;

void func_800ADF34(void);
void func_800A3938(s32 sound_id, s32 pan);
void func_800A788C(void);
void func_800A7B54(void);

/**
 * @brief Initialize two field processes, positioning the second by actor count.
 * @note Partial match: 85.700000% with gcc272_cdk; scheduling differences remain.
 * @see decomp.me WIP
 */
void func_800A7724(void)
{
    FieldProcEntry *rec;
    Entry268 *entry;
    s32 i;
    s32 count;
    s32 packed;
    u32 mask;
    s32 state_mask;

    func_800ADF34();
    D_800F229C = 1;
    func_800A3938(0xB9, 0x80);

    count = 0;
    rec = func_800ADF84();
    i = count;
    rec->update = func_800A788C;
    rec->attributes.flags = (rec->attributes.flags & -0x79) | 8;
    rec->attributes.flags = rec->attributes.flags & 0xFFFF007F;
    rec->attributes.flags = rec->attributes.flags | 0x1000;
    rec->attributes.bytes.position = 0x30;
    rec->state = (rec->state | 1) & -0x1FF;
    rec->state = rec->state | 0x40;
    rec->attributes.flags = rec->attributes.flags & 0xFFFFFF;

    entry = D_800FD818;
    for (i = 0; i < 3; i++)
    {
        if (entry->unk0 & 1)
        {
            count++;
        }
        entry = (Entry268 *)((u8 *)entry + 0x268);
    }

    rec = func_800ADF84();
    rec->update = func_800A7B54;
    rec->attributes.flags = (rec->attributes.flags & -0x79) | 8;
    rec->attributes.flags = rec->attributes.flags & 0xFFFF007F;
    rec->attributes.flags = rec->attributes.flags | 0x1000;
    mask = 0xFFFFFF;
    rec->attributes.bytes.position = 0x58;
    /* Preserve the compiler boundary around the packed position calculation. */
    do
    {
        packed = (count << 3) - count;
        packed = packed << 2;
        packed = packed + 0x10;
        packed = packed & 0xFF;
        packed = packed << 1;
    } while (0);
    rec->state = (rec->state | 1) & -0x1FF;
    rec->state = rec->state | packed;
    rec->attributes.flags = rec->attributes.flags & mask;
}
