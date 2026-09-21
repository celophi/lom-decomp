#include "common.h"

#include "sdk/libgte.h"

/** @brief Eight bytes of effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_80076DBC(void);
extern WmapBlk8 D_80139258;
extern s32 D_801B2474;
extern WmapBlk8 D_801B24A8;
extern s32 D_801B2600;
extern s32 D_801B2604;
extern VECTOR D_801B2478;

/** @brief Restore effect data, reset its vector, and begin a 16-tick sequence step. */
void func_80077DE8(void)
{
    D_801B24A8 = D_80139258;
    D_801B2474 = 0x80;
    D_801B2478.vz = 0xC350;
    D_801B2478.vy = 0;
    D_801B2478.vx = 0;
    D_801B2604 = 0x10;
    D_801B2600 += 1;
    func_80076DBC();
}
