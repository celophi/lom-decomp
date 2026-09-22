#include "common.h"

typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern s32 D_801B2B54;
extern u32 D_801B2B50;
extern void func_80093548(void);

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80094C68(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2B54 = 0x80;
    D_801B2B50 += 1;
    func_80093548();
}
