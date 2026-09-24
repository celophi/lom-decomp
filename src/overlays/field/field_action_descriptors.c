#include "game_audio.h"
#include "common.h"
#include "field_records.h"

extern FieldBattleContext *D_80123FB0;
extern u8 *D_80123FAC;
extern u8 *D_80122B74;

extern void *func_800B543C(s32);
extern void *func_800C2958(s32, u16);

/**
 * @brief Select the active action descriptor and initialize the power fields.
 * @return The descriptor for the attacker's current action, or NULL when there is none.
 */
void *func_800B50B8(void)
{
    s32 magic_index;
    /* Also reused for the 0x250 character offset; separate locals change registers (97.60%). */
    s32 action_id;
    s32 kind;
    u32 selector;
    s32 entry;
    s32 slot_offset;
    u8 *table;
    u8 *character;
    u8 *item;
    u8 *descriptor;
    u8 *base;
    FieldStatusRecord *attacker;
    FieldStatusRecord *caster;
    FieldBattleAction *action;
    FieldActorTemplate *template;
    FieldBattleContext *ctx;

    D_80123FB0->power = D_80123FB0->attacker->unk18;
    D_80123FB0->power_flags = 0U;
    attacker = D_80123FB0->attacker;
    kind = attacker->meta.packed & 0xFC00;
    if (kind == 0x1400)
    {
        action = D_80123FB0->action;
        template = attacker->template;
        action_id = action->action_id;
        if (action_id < template->action_count)
        {
            descriptor = template->actions[action_id];
        }
        else if ((action_id != 0x17) && (action_id != 0x18))
        {
            record_game_diagnostic(0x8001, 0x69, action->attacker_id, action_id);
            descriptor = NULL;
        }
        else
        {
            descriptor = NULL;
        }
    }
    else if ((kind == 0xC00) || (kind == 0x1000))
    {
        descriptor = func_800C2958(2, (u16)D_80123FB0->action->action_id);
    }
    else
    {
        s32 descriptor_base;
        descriptor_base = (s32) D_80122B74;
        character = (u8 *) ((attacker->meta.bytes.id * 0x250) + descriptor_base + 0x640);
        D_80123FB0->power_flags = (u8) *(u8 *)(character + 0x2C);
        selector = D_80123FB0->action->action_id;
        switch (selector)
        {
        case 0:
        case 1:
            descriptor = func_800B543C(*(D_80122B74 - (-(D_80123FB0->action->action_id + D_80123FB0->attacker->meta.bytes.id * 0x250)) + 0x60A));
            break;
        case 2:
            table = D_80123FAC;
            table += *(s32 *)(table + 0x0);
            descriptor = table + (*(u8 *)(character + 0x26) * 8);
            descriptor += D_80123FB0->action->param * 8;
            break;
        case 3:
            descriptor = D_80123FAC + *(s32 *)(D_80123FAC + 0x0) + (character[D_80123FB0->action->action_id + 0x24] * 8);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            ctx = D_80123FB0;
            base = D_80122B74;
            action_id = ctx->attacker->meta.bytes.id * 0x250;
            slot_offset = action_id - 4;
            entry = *(base + (ctx->action->action_id + slot_offset) + 0x60C);
            if (entry < 0x80U)
            {
                {
                    s32 low_index;
                    low_index = ((((u32) *(u32 *)(character + 0x14) >> 0xA) & 0x3F) * 0x18) + entry;
                    descriptor = D_80123FAC + *(s32 *)(D_80123FAC + 0x4) + (low_index << 3);
                }
            }
            else
            {
                item = base + (action_id + 0x5F0) + ((((entry + 4) & 0x7F) << 6) + 0x50);
                entry = *(u8 *)(item + 0x26);
                ctx->power_flags = 0U;
                ctx->power = (u16) entry;
                descriptor = D_80123FAC + *(s32 *)(D_80123FAC + 0x8) + (*(u8 *)(item + 0x24) * 0x70) + (*(u8 *)(item + 0x25) * 8);
            }
            break;
        case 8:
        case 9:
        case 10:
            magic_index = D_80123FB0->action->action_id;
            descriptor = D_80123FAC + *(s32 *)(D_80123FAC + 0x0) + (character[magic_index + 0x20] * 8);
            if ((u32) (magic_index - 0x12) < 2U)
            {
                caster = D_80123FB0->attacker;
                caster->counter = caster->counter - 1;
            }
            break;
        default:
            descriptor = (D_80123FAC + *(s32 *)(D_80123FAC + 0xC) + (selector * 8)) - 0x38;
            break;
        }
    }
    return descriptor;
}



typedef struct
{
    u8 pad0[0xC];
    u32 unkC;
} TrackBaseB543C;




/**
 * @brief Compute the next command-stream pointer for a field command opcode.
 * @param opcode Field command opcode.
 * @return Pointer to the next command, or NULL when processing does not continue.
 */
void* func_800B543C(s32 opcode)
{
    void* result;

    result = (u8*)D_80123FAC + ((TrackBaseB543C *)D_80123FAC)->unkC;
    switch (opcode)
    {
    case 0x33:
        func_800B61EC(D_80123FB0->attacker->meta.bytes.id);
        result = (u8*)result + 0x80;
        break;
    case 0x34:
        result = (u8*)result + 0x78;
        D_80123FB0->attacker->counter -= 1;
        break;
    case 0x32:
        break;
    case 0x36:
        result = (u8*)result + 8;
        break;
    case 0x37:
        result = NULL;
        break;
    case 0x3E:
        result = (u8*)result + 0x10;
        break;
    case 0x43:
        result = (u8*)result + 0x18;
        break;
    case 0x45:
        result = (u8*)result + 0x20;
        break;
    case 0x4F:
        result = (u8*)result + 0x70;
        break;
    default:
        record_game_diagnostic(0x8001, 0x66, opcode, -1);
        return NULL;
    }
    return result;
}
