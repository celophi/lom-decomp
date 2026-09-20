#ifndef CONTROLLER_INTERNAL_H
#define CONTROLLER_INTERNAL_H

#include "common.h"

typedef void (*VSyncCallbackFn)(void);

/** @brief Saved VSync callback as an SDK return value or callable handler. */
typedef union
{
    s32 address;
    VSyncCallbackFn handler;
} ControllerVSyncCallback;

extern u8 g_controller_vsync_sample_count;
extern u8 g_controller_vsync_counter;
extern ControllerVSyncCallback g_previous_controller_vsync_callback;

/**
 * @brief Game-facing controller device categories.
 */
typedef enum ControllerDeviceType
{
    CONTROLLER_DEVICE_DIGITAL = 0,
    CONTROLLER_DEVICE_ANALOG_JOYSTICK = 1,
    CONTROLLER_DEVICE_ANALOG = 2,
    CONTROLLER_SUPPORTED_DEVICE_TYPE_COUNT = 3,
    CONTROLLER_DEVICE_CONFIGURING = 0xFE,
    CONTROLLER_DEVICE_DISCONNECTED = 0xFF
} ControllerDeviceType;

/**
 * @brief Controller classes encoded in the high nibble of a LIBPAD packet ID.
 */
typedef enum ControllerPacketType
{
    CONTROLLER_PACKET_DIGITAL = 4,
    CONTROLLER_PACKET_ANALOG_JOYSTICK = 5,
    CONTROLLER_PACKET_DUALSHOCK = 7
} ControllerPacketType;

enum
{
    CONTROLLER_PORT_COUNT = 2,
    CONTROLLER_SECOND_PORT_FLAG = 0x10,
    CONTROLLER_MULTITAP_SLOT_MASK = 0xF,
    CONTROLLER_MULTITAP_SLOT_COUNT = 4,
    CONTROLLER_MULTITAP_HEADER_SIZE = 2,
    CONTROLLER_MULTITAP_SLOT_STRIDE = 8,
    CONTROLLER_RECEIVE_BUFFER_SIZE = CONTROLLER_MULTITAP_HEADER_SIZE + (CONTROLLER_MULTITAP_SLOT_COUNT * CONTROLLER_MULTITAP_SLOT_STRIDE),
    CONTROLLER_MULTITAP_PACKET_ID = 0x8000,
    CONTROLLER_DISCONNECTED_FLAG = 0x100,
    CONTROLLER_ACTUATOR_SETUP_MASK = 0x600,
    CONTROLLER_ACTUATOR_SETUP_MODE = 0x200,
    CONTROLLER_ACTUATOR_SETUP_ACTIVE = 0x400,
    CONTROLLER_USE_DEFAULT_ANALOG_CENTER = 0x800,
    CONTROLLER_ACTUATOR_RUNTIME_FLAGS_MASK = 0xF00,
    CONTROLLER_ACTUATOR_ALIGNMENT_COUNT = 6,
    CONTROLLER_ACTUATOR_UNMAPPED = 0xFF,
    CONTROLLER_LEGACY_VIBRATION_DEVICE_ID = 0x40,
    CONTROLLER_LEGACY_ACTUATOR_CURRENT = 10,
    CONTROLLER_SMALL_MOTOR_ENABLE_FLAG = 1,
    CONTROLLER_ANALOG_CENTER = 0x80,
    CONTROLLER_ANALOG_DEADZONE = 0x38,
    CONTROLLER_ANALOG_DEADZONE_WIDTH = (CONTROLLER_ANALOG_DEADZONE * 2) + 1,
    CONTROLLER_ANALOG_MIN = -CONTROLLER_ANALOG_CENTER,
    CONTROLLER_ANALOG_MAX = CONTROLLER_ANALOG_CENTER - 1,
    CONTROLLER_ANALOG_SCALE_SHIFT = 4,
    CONTROLLER_ANALOG_DIRECTION_HELD_MASK = 0xF,
    CONTROLLER_ANALOG_DIRECTION_EVENT_SHIFT = 4,
    CONTROLLER_ACTUATOR_CYCLE_STEPS = 16,
    CONTROLLER_ACTUATOR_CYCLE_MASK = CONTROLLER_ACTUATOR_CYCLE_STEPS - 1,
    CONTROLLER_FAST_REPEAT_DELAY = 11,
    CONTROLLER_FAST_REPEAT_INTERVAL = 3,
    CONTROLLER_NORMAL_REPEAT_DELAY = 22,
    CONTROLLER_NORMAL_REPEAT_INTERVAL = 6,
    CONTROLLER_MAX_PENDING_SAMPLES = 3,
    CONTROLLER_ACTUATOR_SETUP_READY = 2
};

#define CONTROLLER_IS_DISCONNECTED(status) (((status) >> 8) & 1)
#define CONTROLLER_ACTUATOR_SETUP_STATE(status) (((status) >> 9) & 3)
#define CONTROLLER_IS_WITHIN_ANALOG_DEADZONE(delta) ((u32)((delta) + CONTROLLER_ANALOG_DEADZONE) < CONTROLLER_ANALOG_DEADZONE_WIDTH)

/**
 * @brief One 16-byte processed controller sample.
 */
typedef struct ControllerSample
{
    u8 device_type;
    u8 analog_direction_bits;
    u16 held_buttons;
    u16 pressed_buttons;
    u16 repeat_buttons;
    s16 right_stick_x;
    s16 right_stick_y;
    s16 left_stick_x;
    s16 left_stick_y;
} ControllerSample;

/**
 * @brief One eight-byte controller response embedded directly or in a multitap packet.
 */
typedef struct ControllerPacket
{
    u8 status;
    u8 id;
    u16 buttons;
    u8 right_stick_x;
    u8 right_stick_y;
    u8 left_stick_x;
    u8 left_stick_y;
} ControllerPacket;

/**
 * @brief Multitap header followed by one fixed-size response per slot.
 */
typedef struct ControllerMultitapPacket
{
    u16 id;
    ControllerPacket slots[CONTROLLER_MULTITAP_SLOT_COUNT];
} ControllerMultitapPacket;

/**
 * @brief LIBPAD receive storage interpreted as either a direct or multitap response.
 */
typedef union ControllerReceiveBuffer
{
    u8 bytes[CONTROLLER_RECEIVE_BUFFER_SIZE];
    u16 packet_id;
    ControllerPacket direct;
    ControllerMultitapPacket multitap;
} ControllerReceiveBuffer;

/** @brief Receive-buffer or individual response view of the current packet address. */
typedef union ControllerPacketView
{
    ControllerReceiveBuffer* receive;
    ControllerPacket* pad;
} ControllerPacketView;

/**
 * @brief Packed large-motor command and actuator setup state.
 */
typedef union ControllerActuatorControl
{
    u16 value;
    struct
    {
        u8 large_motor_command;
        u8 state_flags;
    } fields;
} ControllerActuatorControl;

/**
 * @brief Per-port samples, repeat state, and actuator configuration.
 */
typedef struct ControllerPortState
{
    ControllerSample published_sample;
    ControllerSample accumulated_sample;
    ControllerSample current_sample;
    ControllerSample frame_samples[CONTROLLER_MAX_PENDING_SAMPLES];
    ControllerSample vsync_samples[CONTROLLER_MAX_PENDING_SAMPLES];
    u8 actuators_enabled;
    u8 small_motor_command;
    ControllerActuatorControl actuator_control;
    u8 legacy_vibration_device_id;
    u8 actuator_values[3];
    u8 actuator_alignment[CONTROLLER_ACTUATOR_ALIGNMENT_COUNT];
    u8 face_repeat_timer_up;
    u8 face_repeat_timer_right;
    u8 face_repeat_timer_down;
    u8 face_repeat_timer_left;
    u8 direction_repeat_timer_up;
    u8 direction_repeat_timer_right;
    u8 direction_repeat_timer_down;
    u8 direction_repeat_timer_left;
    u8 right_stick_center_x;
    u8 right_stick_center_y;
    u8 left_stick_center_x;
    u8 left_stick_center_y;
    u8 actuator_count;
    u8 small_motor_current;
    u8 large_motor_current;
    u8 port_id;
} ControllerPortState;

/**
 * @brief State for both controller ports and their LIBPAD receive buffers.
 */
typedef struct ControllerState
{
    ControllerPortState ports[CONTROLLER_PORT_COUNT];
    ControllerReceiveBuffer receive_buffers[CONTROLLER_PORT_COUNT];
    u8 published_sample_count;
    u8 pending_sample_count;
    u8 vsync_accumulation_count;
    u8 vsync_accumulation_interval;
    ControllerVSyncCallback previous_vsync_callback;
    u8 fast_button_repeat;
    u8 actuator_cycle;
    u8 sample_unavailable;
} ControllerState;

#define CONTROLLER_STATE ((ControllerState*)0x801ED600)

#endif
