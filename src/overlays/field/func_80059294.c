#include "common.h"

typedef struct {
    u8 _pad[0x34];
    void *unk34;
} FieldScene;

typedef struct {
    FieldScene *scene;
    u32 vram_byte_count;
} FieldSceneGlobals;

extern FieldSceneGlobals g_field_scene;

/**
 * @brief Push arg0 onto the linked list rooted at g_field_scene.scene->unk34,
 *        storing the previous head into *arg0.
 * @param arg0 Pointer to the next-pointer field of the node being inserted.
 * @see decomp.me (98.75%) TODO
 */
void func_80059294(void **arg0) {
    FieldScene *scene = g_field_scene.scene;
    *arg0 = scene->unk34;
    scene->unk34 = arg0;
}
