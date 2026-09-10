#include "common.h"

/** @brief Actor resource entry with a 0x54-byte stride. */
typedef struct
{
    u8 pad0[0x1C];
    s32 unk1c;
    u8 unk20;
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 pad25[2];
    u8 unk27;
    u8 pad28[2];
    s16 unk2a;
    u8 pad2c[2];
    u16 unk2e;
    u16 unk30;
    u8 pad32[0x22];
} FieldTransitionEntry;

/** @brief Sparse actor state with a 0x23C-byte stride. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkc;
    u8 pad10[0x164];
    s32 unk174;
    volatile s32 unk178;
    u8 pad17c[0x11];
    u8 unk18d;
    u8 pad18e[0x1d];
    u8 unk1ab;
    u8 pad1ac[0x90];
} FieldTransitionActor;

/** @brief Controller slot state with a 0x268-byte stride. */
typedef struct
{
    u8 pad0[0x25A];
    u8 unk25a, unk25b, unk25c, unk25d;
    u8 pad25e[10];
} FieldTransitionSlot;

/** @brief Actor part state with a 0x48-byte stride. */
typedef struct
{
    u8 pad0[0x34];
    s32 unk34;
    u8 pad38[0x10];
} FieldTransitionPart;
extern void func_8006C3FC(FieldTransitionEntry *);
extern void func_80083BC0(FieldTransitionEntry *, void *, s32);
extern void func_80086494(s32);
extern void func_8008A0B0(FieldTransitionEntry *, s32, s32);
extern void func_80092124(void);
extern void func_80096B54(void);
extern void func_800A2DD8(s32);
extern void func_800A6204(void);
extern void func_800AB710(void);
extern void func_800B0234(void);
extern void func_800B34D0(s32);
extern u8 D_800FB3C8[];
extern FieldTransitionSlot D_800FD818;
extern FieldTransitionEntry D_800FDF58;
extern FieldTransitionPart D_800FE3A0;
extern s32 D_800FE754;
extern FieldTransitionActor D_80105AE0;
extern s32 D_8010AE54, D_8010AE5C, D_8010CFD0, D_8010D020, D_8011F420, D_8012291C;
extern u32 D_801229A0;
extern u8 g_field_actor_slots[];
extern u8 *g_pad_ctx;

/**
 * @brief Reset field actors or prepare their state for a field transition.
 * @param mode Zero resets the three actor slots; nonzero prepares transition state.
 * @param actor_data Actor data supplied by lifecycle callers; unused here.
 * @note GCC 2.7.2 CDK match: 92.362240%, 158/196 exact instructions.
 * @note Explicit loops preserve the original pointer lifetimes; both flag stores
 *       at offset 0x178 are retained through the volatile field.
 */
void func_800966F0(s32 mode, void *actor_data)
{
    FieldTransitionActor *actor;
    FieldTransitionEntry *entry;
    FieldTransitionSlot *slot;
    s32 animation;
    s32 flags;
    s32 pad_offset;
    s32 index;
    s32 actor_offset;
    s32 template_offset;
    u32 *saved_buttons;
    u32 buttons;
    u8 *slot_bytes;
    u8 animation_flags;
    void *actor_template;
    u8 *actor_slot;
    u8 *actor_base;
    u8 **pad_base;
    FieldTransitionEntry *entry_arg;
    s32 force, mask;
    FieldTransitionEntry *companion;
    FieldTransitionPart *part;

    func_800A6204();
    if (mode == 0)
    {
        index = 0;
        func_80096B54();
        D_8010AE5C = 0;
        func_800B34D0(0);
        actor_base = g_field_actor_slots;
        template_offset = index;
        entry = &D_800FDF58;
        actor_offset = 0x9100;
        actor = &D_80105AE0;
        D_8010AE54 = 1;
    reset_actor:
    {
        mask = ~0x8000;
        entry_arg = entry;
        force = 1;
        actor_template = template_offset + D_800FB3C8;
        template_offset += 0x244;
        actor_slot = actor_offset + actor_base;
        actor_offset += 0x244;
        actor->unk1ab = 0;
        actor->unkc = (s32)(actor->unkc & 0x200);
        actor->unk174 = (s32)(actor->unk174 & mask);
        flags = actor->unk178 & ~0x20;
        actor->unk178 = flags;
        actor->unk178 = (s32)(flags & ~0x80);
        actor_slot[0x225] = 0;
        func_80083BC0(entry_arg, actor_template, force);
        actor->unk18d = 0;
        entry->unk30 = 0;
        func_800A2DD8(index);
        func_80086494(index);
        entry = (FieldTransitionEntry *)((u32)entry + 0x54);
        index += 1;
        actor = (FieldTransitionActor *)((u32)actor + 0x23C);
    }
        if (index < 3)
        {
            goto reset_actor;
        }
        D_8010CFD0 = 0;
        return;
    }
    if (D_8010D020 != 0)
    {
        func_800AB710();
    }
    D_800FE754 = mode;
    func_80092124();
    index = 0;
    pad_base = &g_pad_ctx;
    slot = &D_800FD818;
    saved_buttons = &D_801229A0;
    pad_offset = index;
    do
    {
        index += 1;
        buttons = *(u32 *)(*pad_base + pad_offset + 0x610);
        pad_offset += 0x250;
        *saved_buttons = buttons >> 8;
        slot->unk25d = 0;
        slot->unk25c = 0;
        slot->unk25b = 0;
        slot->unk25a = 0;
        slot = (FieldTransitionSlot *)((u32)slot + 0x268);
        saved_buttons++;
    } while (index < 3);
    D_8012291C = 1;
    D_8011F420 = *(s32 *)(g_pad_ctx + 0x2C);
    func_800B0234();
    D_800FDF58.unk24 = 1;
    D_800FDF58.unk2e = 1;
    D_800FDF58.unk27 = 0;
    D_800FDF58.unk1c = (s32)(D_800FDF58.unk1c & ~0x800);
    animation = D_800FDF58.unk21 & 0x7F;
    animation %= 5;
    animation_flags = (D_800FDF58.unk21 & 0x80) + animation;
    D_800FDF58.unk21 = animation_flags;
    func_8006C3FC(&D_800FDF58);
    index = 1;
    if (D_8010D020 == 0)
    {
        part = &D_800FE3A0;
        part = (FieldTransitionPart *)((u32)part + index * 0x48);
        companion = (FieldTransitionEntry *)((u32)&D_800FDF58 + 0x54);
        slot_bytes = (u8 *)&D_800FD818;
        slot_bytes += index * 0x268;
    reset_companion:
    {
        if (*slot_bytes & 1)
        {
            if (*(u16 *)&companion->unk1c & 0x1FF)
            {
                companion->unk2a = 0xAF;
                companion->unk2e = 0xFFFF;
                part->unk34 = (s32)(part->unk34 | 0x800000);
            }
            else
            {
                func_8008A0B0(companion, 0, 1);
                part->unk34 = (s32)(part->unk34 | 0x800000);
                if (companion->unk2a == 0xB5)
                {
                    companion->unk2a = 0xB1;
                }
            }
        }
        part = (FieldTransitionPart *)((u32)part + 0x48);
        companion = (FieldTransitionEntry *)((u32)companion + 0x54);
        index += 1;
        slot_bytes += 0x268;
    }
        if (index < 3)
        {
            goto reset_companion;
        }
    }
}
