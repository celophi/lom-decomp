#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/libetc.h"
#include "cdrom.h"

extern s32 D_800D921C;
extern s32 D_800DBE7C;
extern s32 D_800DCEDC;
extern u8 D_8010CF18[];
extern u8 D_80114F18[];
extern s32 D_8011CF74;
/** @brief Frame-buffer storage with a primitive cursor at offset 0x33C. */
typedef struct
{
    u8 pad_00[0x33C];
    u8 *cursor;
    u8 pad_340[0x7B00];
} WmapFrame;

extern WmapFrame D_80129560[2];
extern u8 *D_801398EC;
extern void func_8006CB60(void);

/** @brief Select the back buffer, run effects, and submit a world-map frame. */
void func_80064AF8(void)
{
    if (D_8011CF74 & 1)
    {
        D_80129560[0].cursor = D_8010CF18;
        D_801398EC = (u8 *)&D_80129560[0];
    }
    else
    {
        D_80129560[1].cursor = D_80114F18;
        D_801398EC = (u8 *)&D_80129560[1];
    }
    D_800D921C = 0;
    ClearOTagR((u_long *)(D_801398EC + 0x70), 179);
    D_800DBE7C = 0;
    D_8011CF74++;
    func_8006CB60();
    DrawSync(0);
    VSync(4);
    PutDispEnv((DISPENV *)(D_801398EC + 0x5C));
    PutDrawEnv((DRAWENV *)D_801398EC);
    SetGeomScreen(D_800DCEDC);
    DrawOTag((u_long *)(D_801398EC + 0x338));
    cdrom_process_state();
    DrawSync(0);
}
