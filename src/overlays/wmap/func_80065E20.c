#include "wmap_effect_primitives.h"
/* Partial WMAP decompilation: 91.246750% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Byte-aligned source vertex record. */
typedef struct
{
    u8 bytes[8];
} WmapVertex;
/** @brief Textured quad with packed coordinate words. */
typedef struct
{
    u32 tag, color;
    s32 xy0;
    u32 uv0;
    s32 xy1;
    u32 uv1;
    s32 xy2;
    u32 uv2;
    s32 xy3;
    u32 uv3;
} WmapQuad;
extern WmapVertex D_80051D4C[];
extern s16 D_80053414[];
extern u8 *D_801398EC;
extern void func_80065F54(void);

/** @brief Project grid vertices into the four adjacent textured quads. */
void func_80065E20(void)
{
    WmapVertex position;
    s32 screen_position;
    s32 row;
    u16 column;
    s32 vertex_index;
    WmapQuad *top_left;
    WmapQuad *top_right;
    WmapQuad *bottom_left;
    WmapQuad *bottom_right;

    func_8006AEE0();
    for (row = 0; row < 25; row++)
    {
        top_left = (WmapQuad *)(D_801398EC + 0x340) + row * 26;
        top_right = top_left + 1;
        bottom_left = top_left + 26;
        bottom_right = top_left + 27;
        vertex_index = (row + 1) * 4;
        for (column = 0; column < 25;)
        {
            position = D_80051D4C[D_80053414[vertex_index]];
            gte_ldv0(&position);
            gte_rtps();
            column++;
            vertex_index = column * 104 + (row + 1) * 4;
            gte_stsxy(&screen_position);
            bottom_right->xy0 = screen_position;
            bottom_left->xy1 = screen_position;
            top_right->xy2 = screen_position;
            top_left->xy3 = screen_position;
            top_left++;
            top_right++;
            bottom_left++;
            bottom_right++;
        }
    }
    func_80065F54();
}
