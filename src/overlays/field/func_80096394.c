#include "common.h"

/** @brief Presence byte in a 0x54-byte field actor record. */
typedef struct
{
    u8 pad[0x25];
    u8 presence;
    u8 tail[0x2E];
} FieldColorActor;
/** @brief Links, state, base color, and flash timer in a 0x23C-byte actor slot. */
typedef struct
{
    u32 unknown0;
    u32 link;
    u8 pad8[0x16C];
    u32 options;
    union
    {
        u32 word;
        struct
        {
            unsigned bit0 : 1;
            unsigned low : 4;
            unsigned bit5 : 1;
            unsigned bit6 : 1;
            unsigned bit7 : 1;
            unsigned high : 24;
        } bits;
    } state;
    u8 pad17c[0x2C];
    u8 red, green, blue, timer;
    u8 tail[0x90];
} FieldColorActorSlot;
/** @brief Rendered color in a 0x48-byte field visual record. */
typedef struct
{
    u8 pad[0xE];
    u8 red, green, blue;
    u8 tail[0x37];
} FieldColorVisual;
/** @brief Active state and actor index in a 0x1C-byte selection record. */
typedef struct
{
    s32 active;
    u8 pad4[8];
    s32 actor_index;
    u8 tail[0xC];
} FieldColorSelection;
extern FieldColorActor D_800FDF58[];
extern FieldColorActorSlot D_80105AE0[];
extern FieldColorVisual D_800FE3A0[];
extern FieldColorSelection D_80105880[];
extern s32 D_800FE754;
extern s32 g_frame_counter;

/**
 * @brief Update actor tint colors and timed or selection-dependent flashing.
 * @note The thirteen slots retain separate byte and word reads of state flags.
 * @note A set timer/frame bit dims each base color to 100/128 of its value.
 */
void func_80096394(void)
{
    s32 i = 0;
    FieldColorVisual *visual = D_800FE3A0;
    FieldColorActor *actor = D_800FDF58;
    FieldColorActorSlot *slot = D_80105AE0;
    s32 selected;
    u32 flags;
    u32 options;
    u8 timer;

    do
    {
        slot = &D_80105AE0[i];
        visual = &D_800FE3A0[i];
        if (D_800FDF58[i].presence != 0xFF)
        {
            timer = slot->timer;
            if (timer != 0)
            {
                if (timer & 4)
                {
                    visual->red = (slot->red * 100) / 128;
                    visual->green = (slot->green * 100) / 128;
                    visual->blue = (slot->blue * 100) / 128;
                }
                else
                {
                    visual->red = slot->red;
                    visual->green = slot->green;
                    visual->blue = slot->blue;
                }
                timer = slot->timer - 1;
                slot->timer = timer;
                if (timer == 0)
                {
                    slot->options &= 0xFFFF7FFF;
                }
            }
            else
            {
                if (slot->link != 0 && !(*(u8 *)&slot->state.word & 1))
                {
                    flags = slot->state.word;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1))
                    {
                        if (!((flags >> 6) & 1))
                        {
                            selected = i;
                            if (i >= 3)
                            {
                                selected = 2;
                            }
                            if (D_80105880[selected].actor_index == i)
                            {
                                selected = i;
                                if (i >= 3)
                                {
                                    selected = 2;
                                }
                                if (D_80105880[selected].active != 0)
                                {
                                    goto check_blink;
                                }
                            }
                        }
                        slot->options &= 0xFFFF7FFF;
                        visual->red = slot->red;
                        goto restore_green;
                    }
                }
check_blink:
                options = slot->options & 0xFFFF7FFF;
                slot->options = options;
                if (D_800FDF58[i].presence != 0xFF && slot->link != 0 && !(*(u8 *)&slot->state.word & 1))
                {
                    flags = slot->state.word;
                    if (!((flags >> 5) & 1) && !((flags >> 7) & 1) && D_800FE754 != 0)
                    {
                        slot->options = options | 0x8000;
                        if (g_frame_counter & 4)
                        {
                            D_800FE3A0[i].red = (slot->red * 100) / 128;
                            D_800FE3A0[i].green = (slot->green * 100) / 128;
                            D_800FE3A0[i].blue = (slot->blue * 100) / 128;
                        }
                        else
                        {
                            visual->red = slot->red;
restore_green:
                            visual->green = slot->green;
                            visual->blue = slot->blue;
                        }
                    }
                }
            }
        }
        actor++;
        i++;
    } while (i < 13);
}
