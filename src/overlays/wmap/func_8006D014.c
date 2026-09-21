#include "common.h"

extern s32 *D_80139280;
extern void func_8006A2FC(void *, void *, s32, s32, s32, s32, s32, s32 *);
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;

/**
 * @brief Copy the global effect parameters into the descriptor and draw it.
 * @param actor Actor configuration passed to the drawing helper.
 * @param resource Resource slot passed to the drawing helper.
 * @param arg2 TODO: drawing parameter meaning unknown.
 * @param arg3 TODO: drawing parameter meaning unknown.
 * @param arg4 TODO: drawing parameter meaning unknown.
 * @param arg5 TODO: drawing parameter meaning unknown.
 * @param arg6 TODO: drawing parameter meaning unknown.
 */
void func_8006D014(void *actor, void *resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6)
{
    D_80139280[1] = D_80139234;
    D_80139280[2] = D_8013923C;
    D_80139280[3] = D_80139240;
    D_80139280[4] = D_8013924C;
    D_80139280[5] = D_80139250;
    D_80139280[6] = D_80139260;
    D_80139280[7] = D_80139264;
    D_80139280[8] = D_80139268;
    D_80139280[9] = D_8013926C;
    D_80139280[10] = D_80139284;
    func_8006A2FC(actor, resource, arg2, arg3, arg4, arg5, arg6, D_80139280);
}
