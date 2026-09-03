#include "common.h"

typedef struct {
    s32 x;
    s32 y;
    s32 z;
    u16 unkC;
    s16 unkE;
    u16 unk10;
} Query;

typedef struct {
    s32 x;
    s32 y;
    s32 z;
} Pos;

s32 func_8005B368(Query *q);
void func_800B22F0(s32 value, s32 entry);
extern s16 D_800FDF82;
extern s32 D_8010D028;
extern s32 g_field_scene_request_pending;

void func_8009A2A4(Pos *arg0)
{
    Query q;
    s32 hit;

    if (D_800FDF82 == 0) {
        q.x = arg0->x;
        q.y = arg0->y;
        q.z = arg0->z;
        q.unkC = 8;
        q.unkE = 0x10;
        q.unk10 = 5;
        hit = func_8005B368(&q);
        if (hit != -1) {
            if (g_field_scene_request_pending == 0) {
                if (D_8010D028 == 0) {
                    func_800B22F0(0, hit | 0x8000);
                }
                D_8010D028 = 1;
                return;
            }
            D_8010D028 = 0;
            return;
        }
        D_8010D028 = 0;
    }
}
