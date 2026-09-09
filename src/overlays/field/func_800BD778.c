#include "common.h"

/** @brief Script request containing actor selection, operation, and value words. */
typedef struct
{
    u16 index;
    u16 operation;
    u32 a, b, c, d, e, f, g;
} Request;
/** @brief Actor fields read or written by the script operations. */
typedef struct
{
    u32 maximum, current, pad8, flags;
    u8 pad10[8];
    u16 flags18;
    u8 pad_1a[0x2E];
    u16 x, y;
    u8 active;
    u8 pad_4d[3];
    u32 a, b, c;
    u8 pad_5c[4];
    u8 channels[4];
} Actor;
extern u8 *g_field_script;
extern Actor *func_80087F0C(s32);
/**
 * @brief Dispatch eight script operations that read or update actor fields.
 * @param unused Unused incoming argument.
 * @param request Actor selector, operation code, and input/output values.
 */
void func_800BD778(s32 unused, Request *request)
{
    s32 index;
    u32 divisor;
    Actor *actor;
    if (request->index == 0xFF)
    {
        index = *g_field_script;
    }
    else
    {
        index = request->index;
    }
    actor = func_80087F0C(index);
    if (actor != (Actor *)-1)
    {
        switch (request->operation)
        {
        case 0:
            actor->a = request->a;
            actor->b = request->b;
            actor->c = request->c;
            break;
        case 1:
            actor->current = request->a;
            actor->maximum = request->b;
            break;
        case 2:
            actor->flags |= request->a;
            actor->flags18 |= (u16)request->b;
            break;
        case 3:
            actor->flags &= request->a;
            actor->flags18 &= (u16)request->b;
            break;
        case 4:
            request->a = actor->x;
            request->b = actor->y;
            request->c = actor->active & 1;
            break;
        case 5:
            request->a = actor->current;
            request->b = actor->maximum;
            divisor = actor->maximum;
            if (divisor != 0)
            {
                request->c = (actor->current * 8) / divisor;
            }
            else
            {
                request->c = 0;
            }
            break;
        case 6:
            actor->channels[0] = request->d;
            actor->channels[1] = request->e;
            actor->channels[2] = request->f;
            actor->channels[3] = request->g;
            break;
        case 7:
            actor->x = request->a;
            break;
        }
    }
}
