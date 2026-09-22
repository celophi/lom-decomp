#include "wmap_map_labels.h"
#include "wmap_effect_resources.h"
#include "common.h"
#include "cdrom.h"

/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 words[4];
} WmapTransform;

extern s32 D_800DCEC0;
extern WmapTransform D_800DCEC8;
extern s32 D_8011CF50;
extern u8 D_8011D538[];
extern s32 D_80139224;
extern WmapTransform D_80139950;
extern s32 D_8013B208;
extern s32 D_8013B288;
extern s32 D_801B2C48;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;

/** @brief Save the projection state and queue resources for the next sequence. */
void func_8009A4B0(void)
{
    D_80139224 = 1;
    D_8013B288 = 0;
    D_8011CF50 = 1;
    D_801B2C48 = 0;
    D_8013B208 = 1;
    func_8005FF88(-1);
    D_800DCEC0 = 0;
    D_800DCEC8 = D_80139950;
    cdrom_queue_read(0x1145, D_8011D538);
    cdrom_queue_read(0x1146, D_8011D538 + 0x2000);
    func_800A8AA8(0x1147);
    func_800A8AF0(0x1148);
    D_801B2C50 = 1;
    D_801B2C4C += 1;
}
