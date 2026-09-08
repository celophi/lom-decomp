#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    u8 pad[0xC];
    u32 tag;
} FieldPrimitiveContext;

/**
 * @brief Builds and links a shaded two-point line primitive.
 * @param packet Primitive-buffer cursor where the line is written.
 * @param context Draw context containing the ordering-table tag.
 * @param colors Two packed endpoint colors used to derive the line shading.
 * @param intensity Fallback endpoint intensity and horizontal length scale.
 * @param x Base horizontal position.
 * @param y Base vertical position.
 * @return Pointer immediately after the emitted primitive, or @p packet when intensity is zero.
 */
u8 *func_80085D30(u8 *packet, FieldPrimitiveContext *context, u32 *colors, s32 intensity, s32 x, s32 y)
{
    u8 first_component;
    u8 second_component;
    s16 line_x;
    s16 line_y;
    u32 first_color;

    if (intensity == 0)
    {
        return packet;
    }

    first_color = colors[0];
    setlen((LINE_G2 *)packet, 4);
    *(u32 *)&((LINE_G2 *)packet)->r0 = first_color;
    setcode((LINE_G2 *)packet, 0x50);

    first_component = ((u8 *)colors)[0];
    second_component = ((u8 *)colors)[4];
    if (first_component == second_component)
    {
        ((LINE_G2 *)packet)->r1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2 *)packet)->r1 = intensity;
    }
    else
    {
        ((LINE_G2 *)packet)->r1 = ~intensity;
    }

    first_component = ((u8 *)colors)[1];
    second_component = ((u8 *)colors)[5];
    if (first_component == second_component)
    {
        ((LINE_G2 *)packet)->g1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2 *)packet)->g1 = intensity;
    }
    else
    {
        ((LINE_G2 *)packet)->g1 = ~intensity;
    }

    first_component = ((u8 *)colors)[2];
    second_component = ((u8 *)colors)[6];
    if (first_component == second_component)
    {
        ((LINE_G2 *)packet)->b1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2 *)packet)->b1 = intensity;
    }
    else
    {
        ((LINE_G2 *)packet)->b1 = ~intensity;
    }

    line_x = x + 0x18;
    ((LINE_G2 *)packet)->x0 = line_x;
    line_y = y + 0x10;
    ((LINE_G2 *)packet)->y1 = line_y;
    ((LINE_G2 *)packet)->y0 = line_y;
    ((LINE_G2 *)packet)->x1 = line_x + ((intensity * 0x23) / 255);

    addPrim(&context->tag, (LINE_G2 *)packet);
    return packet + sizeof(LINE_G2);
}
