#include "common.h"

typedef struct {
    s16 a;
    s16 b;
} Pair;

extern Pair D_800CA8B8;
extern Pair D_800CA8BC;
extern Pair D_800CA8C8;
extern Pair D_800CA8CC;
extern s32 D_800CA8DC;
extern s32 D_800CA8E0;
extern void func_80050944(void);

/**
 * @brief Reset the two WSEL cursor bounding regions to their default min/max
 *        corners and clear their associated counters.
 * @see (100%)
 */
void func_80052154(void)
{
    D_800CA8C8.a = D_800CA8B8.a = -0x40;
    D_800CA8C8.b = D_800CA8B8.b = -0x70;
    D_800CA8DC = 0;
    D_800CA8CC.a = D_800CA8BC.a = 0x70;
    D_800CA8CC.b = D_800CA8BC.b = 0x40;
    D_800CA8E0 = 0;
    func_80050944();
}
