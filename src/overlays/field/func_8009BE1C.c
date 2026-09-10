#include "common.h"

/** @brief Position, flags, and visual slot in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x10];
    union
    {
        u32 word;
        struct
        {
            unsigned low : 16;
            unsigned group : 2;
            unsigned pad18 : 1;
            unsigned render : 4;
            unsigned high : 9;
        } bits;
    } mode;
    u8 pad20[5];
    u8 presence;
    u8 pad26[0x14];
    u8 slot;
    u8 tail[0x19];
} FieldLoadedActor;
/** @brief Packed links, parameters, and state in a 0x23C-byte actor slot. */
typedef struct
{
    u32 base;
    u32 link;
    u32 tag;
    u32 padc;
    u32 flags;
    s32 index;
    u16 id;
    u16 params[16];
    u8 pad3a[0x13A];
    u32 options;
    union
    {
        u32 word;
        struct
        {
            unsigned low : 5;
            unsigned bit5 : 1;
            unsigned bit6 : 1;
            unsigned bit7 : 1;
            unsigned high : 24;
        } bits;
    } state;
    u8 pad17c[0x12];
    u8 unknown18e;
    u8 pad18f[0x19];
    u8 red, green, blue, alpha;
    u8 tail[0x90];
} FieldLoadedActorSlot;
/** @brief Color bytes in a 0x48-byte field visual record. */
typedef struct
{
    u8 pade[0xE];
    u8 red, green, blue;
    u8 tail[0x37];
} FieldLoadedActorVisual;
/** @brief Packed flags, position, and parameters in a 0x30-byte input record. */
typedef struct
{
    s32 flags;
    s32 unknown4;
    union
    {
        u32 word;
        struct
        {
            u16 x, z;
        } halves;
    } position;
    u16 source;
    u16 id;
    u16 params[16];
} FieldActorLoadEntry;
extern FieldLoadedActor D_800FE054[];
extern FieldLoadedActorSlot D_80106194[];
extern FieldLoadedActorVisual D_800FE3A0[];
extern s32 D_800FE774;
extern s32 D_801178B0;
extern void func_800B118C(FieldActorLoadEntry *, s32);
extern void func_8006B4D0(s32, s32);
extern void func_8006C3FC(FieldLoadedActor *);

/**
 * @brief Initialize active actors from a count-prefixed field entry list.
 * @param data Entry count followed by 0x30-byte actor load records.
 * @note Active entries fill consecutive actor slots beginning at slot three.
 * @note GCC 2.7.2 CDK currently matches 94.607140 percent of the target.
 */
void func_8009BE1C(s32 *data)
{
    FieldLoadedActor *actor = D_800FE054;
    FieldLoadedActorSlot *slot = D_80106194;
    FieldActorLoadEntry *entry = (FieldActorLoadEntry *)(data + 1);
    s32 active = 0;
    s32 index = active;
    s32 count = *data;
    s32 i;
    u32 tag;
    u8 blue;

    D_800FE774 = 3;
    if (count > 0)
    {
        do
        {
            slot->tag &= 0x7FFFFFFF;
            func_800B118C(entry, index);
            if (entry->flags < 0)
            {
                func_8006B4D0(active + 3, entry->source + 3);
                if (((u32)entry->flags >> 30) & 1)
                {
                    actor->presence = 0xFE;
                }
                else
                {
                    actor->presence = 0;
                }
                actor->mode.bits.group = (u32)entry->flags >> 28;
                if ((D_801178B0 & 0x7FFF) == 0x13D)
                {
                    actor->mode.bits.group = 0;
                }
                actor->mode.bits.render = (u32)entry->flags >> 24;
                actor->x = entry->position.halves.x << 8;
                actor->z = (entry->position.halves.z & 0x7FF) << 8;
                actor->y = (entry->position.word >> 30) << 8;
                /* Keep the separate packed-bit updates for target codegen. */
                slot->state.bits.bit6 = 0;
                slot->state.bits.bit7 = 0;
                slot->options &= 0xFFFF7FFF;
                i = 0;
                slot->red = D_800FE3A0[actor->slot].red;
                slot->green = D_800FE3A0[actor->slot].green;
                blue = D_800FE3A0[actor->slot].blue;
                slot->alpha = 0;
                slot->unknown18e = 0;
                tag = slot->tag & 0x80FFFFFF;
                slot->state.bits.bit5 = 0;
                slot->tag = tag;
                slot->blue = blue;
                slot->tag = (tag & 0xFF000000) | (slot->base & 0xFFFFFF);
                slot->index = index + 3;
                slot->link = slot->base & 0xFFFFFF;
                slot->id = entry->id;
                slot->flags = entry->flags;
                do
                {
                    slot->params[i] = entry->params[i];
                    i++;
                } while (i < 16);
                func_8006C3FC(actor);
                slot++;
                actor++;
                active++;
                D_800FE774++;
            }
            index++;
            entry++;
        } while (index < count);
    }
}
