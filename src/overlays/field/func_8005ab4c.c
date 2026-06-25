#include "common.h"

typedef struct {
    u8 _pad[4];
    void *unk4;
} FieldScene;

typedef struct {
    FieldScene *scene;
} FieldSceneGlobals;

extern FieldSceneGlobals g_field_scene;

/**
 * @brief Walk the linked list at g_field_scene.scene->unk4 by arg0 steps,
 *        following the first pointer field (unk0) at each node.
 * @param arg0 Number of steps to walk. If 0, returns the list head unchanged.
 * @return Pointer to the node after arg0 steps.
 * @see decomp.me (100%) TODO
 */
void *func_8005AB4C(s32 arg0) {
    void **node;
    s32 var_v1;

    node = (void **)g_field_scene.scene->unk4;
    var_v1 = arg0 - 1;
    if (arg0 != 0) {
        do {
            node = (void **)*node;
            var_v1 -= 1;
        } while (var_v1 != -1);
    }
    return node;
}
