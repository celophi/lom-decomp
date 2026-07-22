#include "common.h"

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Struct_D_800F2268;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
} Struct_D_800F2270;

extern Struct_D_800F2268 D_800F2268;
extern Struct_D_800F2270 D_800F2270;

/**
 * @brief Set the target RGB intensity and transition duration for the field fade.
 * @param red Target red intensity.
 * @param green Target green intensity.
 * @param blue Target blue intensity.
 * @param duration Transition duration in frames.
 * @see decomp.me (100%) TODO
 */
void field_set_fade_target(s16 red, s16 green, s16 blue, s16 duration) {
    D_800F2268.unk0 = red;
    D_800F2270.unk0 = red;
    D_800F2268.unk2 = green;
    D_800F2270.unk2 = green;
    D_800F2268.unk4 = blue;
    D_800F2270.unk4 = blue;
    D_800F2268.unk6 = duration;
}

/**
 * @brief Initialize D_800F2268 to (0xD0, 0x100, 0x100, 5).
 * @see decomp.me (100%) TODO
 */
void func_80067EE4(void) {
    D_800F2268.unk0 = 0xD0;
    D_800F2268.unk2 = 0x100;
    D_800F2268.unk4 = 0x100;
    D_800F2268.unk6 = 5;
}

/**
 * @brief Write four s16 values into the four fields of D_800F2268.
 * @param arg0 Value for unk0.
 * @param arg1 Value for unk2.
 * @param arg2 Value for unk4.
 * @param arg3 Value for unk6.
 * @see decomp.me (100%) TODO
 */
void func_80067F0C(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    D_800F2268.unk0 = arg0;
    D_800F2268.unk2 = arg1;
    D_800F2268.unk4 = arg2;
    D_800F2268.unk6 = arg3;
}

/**
 * @brief Copy D_800F2270 unk0/unk2/unk4 into D_800F2268 and set D_800F2268.unk6 to 5.
 * @see decomp.me (100%) TODO
 */
void func_80067F28(void) {
    D_800F2268.unk6 = 5;
    D_800F2268.unk0 = (u16)D_800F2270.unk0;
    D_800F2268.unk2 = (u16)D_800F2270.unk2;
    D_800F2268.unk4 = (u16)D_800F2270.unk4;
}

/**
 * @brief Copy D_800F2270 unk0/unk2/unk4 into D_800F2268 and store arg0 into D_800F2268.unk6.
 * @param arg0 Value written to D_800F2268.unk6.
 * @see decomp.me (100%) TODO
 */
void func_80067F5C(s16 arg0) {
    D_800F2268.unk6 = arg0;
    D_800F2268.unk0 = (u16)D_800F2270.unk0;
    D_800F2268.unk2 = (u16)D_800F2270.unk2;
    D_800F2268.unk4 = (u16)D_800F2270.unk4;
}

/**
 * @brief Initialize D_800F2268 to the default color/intensity values (0xC0, 0xC0, 0xC0, 5).
 * @see decomp.me (100%) TODO
 */
void func_80067F8C(void) {
    D_800F2268.unk0 = 0xC0;
    D_800F2268.unk2 = 0xC0;
    D_800F2268.unk4 = 0xC0;
    D_800F2268.unk6 = 5;
}
