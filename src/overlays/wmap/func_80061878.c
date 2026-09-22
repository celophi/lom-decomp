/* Partial WMAP decompilation: 96.412840% (gcc280_g0). */
#include "common.h"
#include "sdk/libgpu.h"

/** @brief Buffer header containing ordering-table and packet allocation state. */
typedef struct
{
    u8 pad_000[0x330];
    u32 order_tag;
    u8 pad_334[8];
    POLY_FT4 *packet;
} WmapPacketBuffer;
extern POLY_FT4 D_800D0694;
extern u8 D_800D0698;
extern s32 D_800D921C;
extern WmapPacketBuffer *D_801398EC;
extern s32 D_8013B254;

/** @brief Update the overlay fade and append its polygon to the ordering table.
 * @return Zero while disabled, otherwise one.
 */
s32 func_80061878(void)
{
    POLY_FT4 *packet;
    s32 *tag_packet;
    u8 intensity;

    switch (D_8013B254)
    {
    case 2:
        intensity = D_800D0694.b0 - 4;
        D_800D0694.b0 = intensity;
        D_800D0694.g0 = intensity;
        D_800D0694.r0 = intensity;
        if (intensity == 0)
        {
            D_8013B254 = 0;
        }
        else
        {
            goto draw;
        }
        break;
    case 1:
        intensity = D_800D0694.b0 + 4;
        D_800D0694.b0 = intensity;
        D_800D0694.g0 = intensity;
        D_800D0694.r0 = intensity;
        if (intensity == 128)
        {
            D_8013B254 = 0;
        }
        break;
    case 3:
        return 0;
    }
    if (D_800D0698 != 0)
    {
draw:
        packet = D_801398EC->packet;
        *packet = D_800D0694;
        if (D_800D0698 != 128)
        {
            packet->code |= 2;
        }
        tag_packet = (s32 *)D_801398EC->packet;
        *tag_packet = (*tag_packet & 0xFF000000) | (D_801398EC->order_tag & 0xFFFFFF);
        D_801398EC->order_tag = (D_801398EC->order_tag & 0xFF000000) | ((u32)D_801398EC->packet & 0xFFFFFF);
        if (D_800D921C < 32000)
        {
            D_800D921C += 40;
            D_801398EC->packet++;
        }
    }
    return 1;
}
