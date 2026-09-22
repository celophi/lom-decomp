#include "wmap_sequence_runtime.h"
#include "common.h"

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

/** @brief Coordinates of the selected map cell. */
typedef struct
{
    s32 x;
    s32 y;
} WmapCoordinates;

extern s16 D_801AFBE0;
extern WmapCoordinates D_8019D248;
extern WmapValueRecord D_80139290[][6];
extern s32 D_801398AC;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_801B2C4C;
extern void func_80099B50(void);
extern void func_8009A75C(void);

/** @brief Schedule the selected cell effect unless its value is two. */
void func_8009A6A8(void)
{
    D_801AFBE0 = 0x3B;
    if (D_80139290[D_8019D248.x][D_8019D248.y].value == 2)
    {
        D_801398AC = 0;
    }
    else
    {
        D_800DCED8 = D_8019D248.x;
        D_800DCEE4 = D_8019D248.y;
        D_801398AC = 1;
        func_8006CBD8(func_80099B50);
    }
    D_801B2C4C++;
    func_8009A75C();
}
