#include "common.h"

#include "sdk/libgte.h"

/** @brief Eight bytes of effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_80076C6C(void);
extern WmapBlk8 D_80139258;
extern s32 D_801B2470;
extern WmapBlk8 D_801B24A0;
extern s32 D_801B25F8;
extern s32 D_801B25FC;
extern VECTOR D_801B2650;

/** @brief Restore effect data, reset its vector, and begin a 48-tick sequence step. */
void func_80077CBC(void)
{
    D_801B24A0 = D_80139258;
    D_801B2470 = 0x80;
    D_801B2650.vz = 0x2710;
    D_801B2650.vy = 0;
    D_801B2650.vx = 0;
    D_801B25FC = 0x30;
    D_801B25F8 += 1;
    func_80076C6C();
}
