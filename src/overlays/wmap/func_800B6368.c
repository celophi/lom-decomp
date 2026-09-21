#include "common.h"

/** @brief Eight-byte transform copied without alignment assumptions. */
typedef struct
{
    u8 bytes[8];
} WmapTransform;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *data;
} WmapResource;

extern s32 *D_80139280;
extern WmapTransform D_80139258;
extern WmapTransform D_801B3120;
extern WmapResource D_80139988[];
extern u8 D_8011F538[];
extern s32 D_801B3078;
extern s32 D_801B307C;
extern void func_800B86BC(void);

/** @brief Configure the effect and its resource slots, then advance the sequence. */
void func_800B6368(void)
{
    s32 i;

    D_80139280[0] = 40;
    D_80139280[1] = 20;
    D_80139280[2] = -210;
    D_80139280[3] = 420;
    D_80139280[4] = -210;
    D_80139280[5] = 420;
    D_80139280[6] = 0;
    D_80139280[7] = 80;
    D_80139280[8] = 129;
    D_80139280[9] = 1;
    D_80139280[10] = 8;
    D_80139280[11] = 80;
    D_80139280[12] = 15;
    D_80139280[13] = 0;
    D_801B3120 = D_80139258;
    for (i = 0; i < 40; i++)
    {
        D_80139988[i + 20].data = D_8011F538;
    }
    D_801B307C = 204;
    D_801B3078++;
    func_800B86BC();
}
