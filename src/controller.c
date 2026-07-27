#include "controller.h"
#include "psyq/libetc.h"
#include "psyq/libpad.h"

typedef void (*VSyncCallbackFn)(void);

extern VSyncCallbackFn g_previous_controller_vsync_callback;
extern u8 g_controller_vsync_sample_count; /* Pending VSync samples at 0x801ED7A1. */
extern u8 g_controller_vsync_counter;      /* Accumulation phase at 0x801ED7A2. */

/**
 * @brief Game-facing controller device categories.
 */
typedef enum ControllerDeviceType
{
    CONTROLLER_DEVICE_DIGITAL = 0,
    CONTROLLER_DEVICE_ANALOG_JOYSTICK = 1,
    CONTROLLER_DEVICE_ANALOG = 2,
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
    CONTROLLER_SECOND_PORT_FLAG = 0x10,
    CONTROLLER_MULTITAP_SLOT_MASK = 0xF,
    CONTROLLER_PACKET_PRESENT = 0x8000,
    CONTROLLER_ACTUATOR_SETUP_MASK = 0x600,
    CONTROLLER_ACTUATOR_SETUP_MODE = 0x200,
    CONTROLLER_ACTUATOR_SETUP_ACTIVE = 0x400,
    CONTROLLER_USE_DEFAULT_ANALOG_CENTER = 0x800,
    CONTROLLER_ACTUATOR_RUNTIME_FLAGS_MASK = 0xF00,
    CONTROLLER_LEGACY_VIBRATION_DEVICE_ID = 0x40,
    CONTROLLER_ANALOG_CENTER = 0x80,
    CONTROLLER_ANALOG_DEADZONE = 0x38,
    CONTROLLER_ANALOG_DEADZONE_WIDTH = 0x71,
    CONTROLLER_ANALOG_MIN = -0x80,
    CONTROLLER_ANALOG_MAX = 0x7F,
    CONTROLLER_ANALOG_SCALE_SHIFT = 4,
    CONTROLLER_ANALOG_NEGATIVE_ROUNDING = 0xF,
    CONTROLLER_FAST_REPEAT_DELAY = 0x0B,
    CONTROLLER_FAST_REPEAT_INTERVAL = 3,
    CONTROLLER_NORMAL_REPEAT_DELAY = 0x16,
    CONTROLLER_NORMAL_REPEAT_INTERVAL = 6,
    CONTROLLER_MAX_PENDING_SAMPLES = 3,
    CONTROLLER_ACTUATOR_SETUP_READY = 2
};

#define CONTROLLER_IS_DISCONNECTED(status) (((status) >> 8) & 1)
#define CONTROLLER_ACTUATOR_SETUP_STATE(status) (((status) >> 9) & 3)
#define CLEAR_CONTROLLER_ACTUATOR_RUNTIME_FLAGS(control) \
    ((control) &= ~CONTROLLER_ACTUATOR_RUNTIME_FLAGS_MASK)

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
    u8 device_type;
    u8 analog_direction_bits;
    u16 held_buttons;
    u16 pressed_buttons;
    u16 repeat_buttons;
    s16 right_stick_x;
    s16 right_stick_y;
    s16 left_stick_x;
    s16 left_stick_y;
    ControllerSample frame_samples[3];
    ControllerSample vsync_samples[3];
    u8 actuators_enabled;
    u8 small_motor_command;
    ControllerActuatorControl actuator_control;
    u8 legacy_vibration_device_id;
    u8 small_motor_value;
    u8 large_motor_value;
    u8 actuator_value_2;
    u8 actuator_alignment[6];
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
    u8 small_motor_power;
    u8 large_motor_power;
    u8 port_id;
} ControllerPortState;

/**
 * @brief State for both controller ports and their LIBPAD receive buffers.
 */
typedef struct ControllerState
{
    ControllerPortState ports[2];
    u8 receive_buffers[2][0x22];
    u8 published_sample_count;
    u8 pending_sample_count;
    u8 vsync_accumulation_count;
    u8 vsync_accumulation_interval;
    VSyncCallbackFn previous_vsync_callback;
    u8 fast_button_repeat;
    u8 actuator_cycle;
    u8 sample_unavailable;
} ControllerState;

#define CONTROLLER_STATE ((ControllerState*)0x801ED600)
#define CONTROLLER_PORT_1_RECEIVE_BUFFER ((u8*)0x801ED75C)
#define CONTROLLER_PORT_2_RECEIVE_BUFFER ((u8*)0x801ED77E)

void controller_poll(void);
void clear_controller_sample(ControllerSample* sample);

extern void PadStartCom();
extern void PadInitDirect(u8* port1_buffer, u8* port2_buffer);
extern s32 PadGetState(s32 port);
extern s32 PadInfoMode(s32 port, s32 info_mode, s32 index);
extern s32 PadInfoAct(s32 port, s32 actuator, s32 property);
extern s32 PadSetActAlign(s32 port, u8* alignment);
extern s32 PadSetMainMode(s32 port, s32 mode, s32 lock);
extern void PadSetAct(s32 port, u8* actuator_data, s32 length);
extern s32 PadChkVsync(void);
extern void PadStopCom(void);
void controller_vsync_callback(void);
void accumulate_controller_sample(ControllerPortState* port);
void merge_latest_controller_sample(ControllerPortState* port);
void copy_controller_sample(ControllerSample* source, ControllerSample* destination);
void poll_controller_port(ControllerPortState* port, s32* actuator_power_total);

/**
 * @brief Initialize LIBPAD, both controller-port records, and their receive buffers.
 * @param enable_actuators Nonzero to enable actuator updates for each port.
 * @see decomp.me (100%) https://decomp.me/scratch/b48Yj
 */
void initialize_controllers(s8 enable_actuators)
{
    ControllerState* controller_state;
    ControllerPortState** current_port_pointer;
    ControllerPortState* current_port;
    ControllerPortState* status_port;
    s32 continue_initializing;
    u16 actuator_control;
    u16 actuator_status;
    s32 port_countdown;
    s32 countdown_end;
    u32 disconnected_device_type;
    s32 legacy_vibration_device_id;
    s32 all_ports_ready;
    s32 status_port_countdown;

    /* Bind LIBPAD to the raw receive buffers and retain callback ownership. */
    PadInitDirect(CONTROLLER_PORT_1_RECEIVE_BUFFER, CONTROLLER_PORT_2_RECEIVE_BUFFER);
    g_previous_controller_vsync_callback = (VSyncCallbackFn)VSyncCallback(0);
    controller_state = CONTROLLER_STATE;

    /* Published and live samples remain disconnected until the first valid poll. */
    controller_state->ports[0].port_id = 0;
    controller_state->ports[1].port_id = CONTROLLER_SECOND_PORT_FLAG;
    clear_controller_sample(&controller_state->ports[0].published_sample);
    clear_controller_sample(&controller_state->ports[1].published_sample);
    clear_controller_sample((ControllerSample*)&controller_state->ports[0].device_type);
    clear_controller_sample((ControllerSample*)&controller_state->ports[1].device_type);
    port_countdown = 1;
    legacy_vibration_device_id = CONTROLLER_LEGACY_VIBRATION_DEVICE_ID;
    disconnected_device_type = CONTROLLER_DEVICE_DISCONNECTED;
    countdown_end = -1;
    current_port = &controller_state->ports[1];

    /* Reset per-port actuator state while preserving unrelated control bits. */
    do
    {
        actuator_control = (*(current_port_pointer = &current_port))->actuator_control.value;
        port_countdown--;
        current_port->legacy_vibration_device_id = legacy_vibration_device_id;
        current_port->actuator_value_2 = 0;
        current_port->large_motor_value = 0;
        current_port->small_motor_value = 0;
        current_port->actuators_enabled = enable_actuators;
        current_port->small_motor_command = 0;
        current_port->actuator_count = 0;
        current_port->small_motor_power = 0;
        current_port->large_motor_power = 0;
        current_port->device_type = disconnected_device_type;
        current_port->published_sample.device_type = disconnected_device_type;
        CLEAR_CONTROLLER_ACTUATOR_RUNTIME_FLAGS(actuator_control);
        current_port->actuator_control.value = actuator_control;
        current_port->actuator_control.fields.large_motor_command = 0;
        current_port--;
    } while (continue_initializing = port_countdown != countdown_end);

    /* Reset frame-level sampling, repeat timing, and actuator cadence. */
    controller_state->fast_button_repeat = 0;
    controller_state->actuator_cycle = 0;
    controller_state->published_sample_count = 0;
    controller_state->pending_sample_count = 0;
    controller_state->sample_unavailable = 0;

    /* Start communication and wait for every port to settle. */
    PadStartCom(disconnected_device_type, port_countdown, countdown_end, legacy_vibration_device_id);
    do
    {
        VSync(0);
        controller_poll();
        all_ports_ready = 1;
        status_port_countdown = all_ports_ready;
        status_port = &controller_state->ports[1];
        do
        {
            actuator_status = (*(current_port_pointer = &status_port))->actuator_control.value;
            if ((!CONTROLLER_IS_DISCONNECTED(actuator_status)) &&
                (CONTROLLER_ACTUATOR_SETUP_STATE(actuator_status) != CONTROLLER_ACTUATOR_SETUP_READY))
            {
                all_ports_ready = 0;
            }
            status_port_countdown--;
            status_port--;
        } while (status_port_countdown != (-1));
    } while (all_ports_ready == 0);

    /* update_controllers() arms VSync accumulation when the frame begins. */
    controller_state->vsync_accumulation_count = 0;
    controller_state->vsync_accumulation_interval = 0;
}

/**
 * @brief Poll one LIBPAD port and update buttons, analog axes, repeat state, and actuators.
 * @param port Per-port controller state to update.
 * @param actuator_power_total Accumulator for the active actuators' current draw.
 * @see decomp.me (100%) https://decomp.me/scratch/rDO0T
 */
void poll_controller_port(ControllerPortState* port, s32* actuator_power_total)
{
    s32 repeat_timer_step;
    ControllerState* controller_state;
    u32 pad_state;
    s32 counter;
    u32 updated_actuator_config;
    s32 mode_index;
    u32 unsigned_value;
    s32 remaining_actuators;
    s32 shifted_delta;
    s32 mode_loop_end;
    s32 multitap_slot;
    u8 disabled_actuator_index;
    s32 dualshock_controller_id;
    s32 fill_loop_end;
    u8 device_type;
    s32 decoded_state;
    u8 initial_repeat_delay;
    s32 repeat_interval;
    u16 held_buttons;
    s32 delta;
    s32 new_analog_directions;
    u32 analog_directions;
    s32 detected_actuator_count;
    u8* pad_packet;
    controller_state = CONTROLLER_STATE;
    pad_state = PadGetState(port->port_id);
    switch (pad_state)
    {
    case PadStateDiscon:
        updated_actuator_config = port->actuator_control.value | 0x100;
        port->device_type = CONTROLLER_DEVICE_DISCONNECTED;
        port->actuator_control.value = updated_actuator_config & 0xF9FF;
        return;

    case PadStateFindPad:
        port->actuator_control.value &= 0xFEFF;
        if (port->actuator_control.value & CONTROLLER_ACTUATOR_SETUP_MASK)
        {
            port->actuator_control.value =
                (port->actuator_control.value & 0xF8FF) | CONTROLLER_ACTUATOR_SETUP_MODE;
        }
        /* Fall through while LIBPAD is negotiating the controller. */

    case PadStateReqInfo:

    case PadStateExecCmd:
        port->device_type = CONTROLLER_DEVICE_CONFIGURING;
        clear_controller_sample((ControllerSample*)&port->device_type);
        return;

    case PadStateFindCTP1:
        /* Legacy analog pads use a fixed device ID and one binary vibration byte. */
        port->legacy_vibration_device_id = CONTROLLER_LEGACY_VIBRATION_DEVICE_ID;
        if (port->actuators_enabled != 0)
        {
            if (port->small_motor_command & 1)
            {
                port->small_motor_value = port->small_motor_command;
                *actuator_power_total += port->small_motor_power;
            }
            else
            {
                u8 large_motor_command = port->actuator_control.fields.large_motor_command;
                if ((large_motor_command != 0) && ((large_motor_command * 0x10) >= controller_state->actuator_cycle))
                {
                    port->small_motor_value = 1;
                    *actuator_power_total += port->small_motor_power;
                }
                else
                {
                    port->small_motor_value = 0;
                }
            }
        }
        else
        {
            port->small_motor_value = 0;
        }
        if ((port->actuator_control.value & CONTROLLER_ACTUATOR_SETUP_MASK) != CONTROLLER_ACTUATOR_SETUP_ACTIVE)
        {
            PadSetAct(port->port_id, &port->legacy_vibration_device_id, 2);
            port->small_motor_power = 10;
            port->actuator_control.value =
                (port->actuator_control.value | CONTROLLER_ACTUATOR_SETUP_ACTIVE) & 0xF5FF;
        }
        break;

    case PadStateStable:
        /* Expanded-protocol pads expose independently aligned actuators. */
        if (port->actuators_enabled != 0)
        {
            u8 large_motor_command = port->actuator_control.fields.large_motor_command;
            if (large_motor_command != 0)
            {
                port->large_motor_value = large_motor_command;
                *actuator_power_total += port->large_motor_power;
            }
            else
            {
                port->large_motor_value = 0;
            }
            if (port->small_motor_command & 1)
            {
                port->small_motor_value = 1;
                *actuator_power_total += port->small_motor_power;
            }
            else
            {
                port->small_motor_value = 0;
            }
        }
        else
        {
            port->small_motor_value = 0;
            port->large_motor_value = 0;
        }
        switch ((port->actuator_control.value >> 9) & 3)
        {
        case 0:
            /* Select DualShock mode before requesting actuator metadata. */
            port->actuator_control.value =
                (port->actuator_control.value & 0xF9FF) | CONTROLLER_ACTUATOR_SETUP_MODE;

            counter = PadInfoMode(port->port_id, InfoModeIdTable, -1);
            mode_index = 0;
            unsigned_value = counter;
            if (unsigned_value != 0)
            {
                counter--;
                dualshock_controller_id = CONTROLLER_PACKET_DUALSHOCK;
                mode_loop_end = -1;
                do
                {
                    if (PadInfoMode(port->port_id, InfoModeIdTable, mode_index) != dualshock_controller_id)
                    {
                        mode_index++;
                        counter--;
                        continue;
                    }
                    if (PadInfoMode(port->port_id, InfoModeCurExOffs, 0) != mode_index)
                    {
                        PadSetMainMode(port->port_id, mode_index, PadModeUnlock);
                        port->device_type = CONTROLLER_DEVICE_CONFIGURING;
                        clear_controller_sample((ControllerSample*)&port->device_type);
                        return;
                    }
                    mode_index++;
                    counter--;
                } while (counter != mode_loop_end);
            }
            /* Fall through to discover and align the controller's actuators. */

        case 1:
            /* Discover actuator capabilities and map them into the transmit buffer. */
            port->actuator_control.value = (port->actuator_control.value & 0xF9FF) | 0xC00;
            counter = 5;
            disabled_actuator_index = 0xFF;
            fill_loop_end = -1;
            while (counter != fill_loop_end)
            {
                port->actuator_alignment[counter] = disabled_actuator_index;
                counter--;
            }

            mode_index++;
            remaining_actuators = PadInfoAct(port->port_id, -1, mode_index = 0);
            detected_actuator_count = remaining_actuators;
            port->actuator_count = detected_actuator_count;
            PadSetAct(port->port_id, &port->small_motor_value, remaining_actuators);
            counter = mode_index;
            remaining_actuators--;
            port->small_motor_power = 0;
            port->large_motor_power = 0;
            while (remaining_actuators != (-1))
            {
                s32 actuator_supported = PadInfoAct(port->port_id, counter, InfoActFunc);
                if (actuator_supported == 1)
                {
                    switch (PadInfoAct(port->port_id, counter, InfoActSize))
                    {
                    case 0:
                        if (port->actuator_alignment[0] == 0xFF)
                        {
                            port->small_motor_power = PadInfoAct(port->port_id, counter, InfoActCurr);
                            port->actuator_alignment[0] = counter;
                        }
                        break;
                    case 1:
                        if (port->actuator_alignment[1] == 0xFF)
                        {
                            port->large_motor_power = PadInfoAct(port->port_id, counter, InfoActCurr);
                            port->actuator_alignment[1] = counter;
                        }
                        break;
                    }
                }
                counter++;
                remaining_actuators--;
            }

            PadSetActAlign(port->port_id, port->actuator_alignment);
            port->device_type = CONTROLLER_DEVICE_CONFIGURING;
            clear_controller_sample((ControllerSample*)&port->device_type);
            return;

        default:
            break;
        }

        break;

    default:
        break;
    }

    /* Select the receive packet for the physical port. */
    if (port->port_id & CONTROLLER_SECOND_PORT_FLAG)
    {
        pad_packet = controller_state->receive_buffers[1];
    }
    else
    {
        pad_packet = controller_state->receive_buffers[0];
    }
    multitap_slot = port->port_id & CONTROLLER_MULTITAP_SLOT_MASK;
    if (multitap_slot != 0)
    {
        /* Multi-tap slots follow a two-byte header in eight-byte records. */
        if ((*((u16*)pad_packet)) == CONTROLLER_PACKET_PRESENT)
        {
            pad_packet += (multitap_slot * 8) + 2;
        }
        else
        {
            port->device_type = CONTROLLER_DEVICE_DISCONNECTED;
            return;
        }
    }
    else if ((*((u16*)pad_packet)) == CONTROLLER_PACKET_PRESENT)
    {
        pad_packet += 2;
    }
    if (pad_packet[0] == 0)
    {
        /* Decode the protocol ID into the game's three supported device classes. */
        u8 controller_id = pad_packet[1];
        u8 controller_class = controller_id >> 4;
        switch (controller_class)
        {
        case CONTROLLER_PACKET_DIGITAL:
            device_type = CONTROLLER_DEVICE_DIGITAL;
            break;
        case CONTROLLER_PACKET_ANALOG_JOYSTICK:
            device_type = CONTROLLER_DEVICE_ANALOG_JOYSTICK;
            break;
        case CONTROLLER_PACKET_DUALSHOCK:
            device_type = CONTROLLER_DEVICE_ANALOG;
            break;
        default:
            device_type = CONTROLLER_DEVICE_DISCONNECTED;
            break;
        }
        decoded_state = device_type;
        if (decoded_state < 3)
        {
            if (decoded_state >= 0)
            {
                held_buttons = ~(*((u16*)(pad_packet + 2)));
                if (port->device_type == decoded_state)
                {
                    port->pressed_buttons = (port->repeat_buttons = held_buttons & (port->held_buttons ^ held_buttons));
                }
                else
                {
                    port->pressed_buttons = (port->repeat_buttons = held_buttons);
                    if (decoded_state == CONTROLLER_DEVICE_ANALOG_JOYSTICK)
                    {
                        port->right_stick_center_x = pad_packet[4];
                        port->right_stick_center_y = pad_packet[5];
                        port->left_stick_center_x = pad_packet[6];
                        port->left_stick_center_y = pad_packet[7];
                        port->analog_direction_bits = 0;
                    }
                    else if (decoded_state == CONTROLLER_DEVICE_ANALOG)
                    {
                        if (!(port->actuator_control.value & CONTROLLER_USE_DEFAULT_ANALOG_CENTER))
                        {
                            port->right_stick_center_x = pad_packet[4];
                            port->right_stick_center_y = pad_packet[5];
                            port->left_stick_center_x = pad_packet[6];
                            port->left_stick_center_y = pad_packet[7];
                            port->analog_direction_bits = 0;
                        }
                        else
                        {
                            port->right_stick_center_x = CONTROLLER_ANALOG_CENTER;
                            port->right_stick_center_y = CONTROLLER_ANALOG_CENTER;
                            port->left_stick_center_x = CONTROLLER_ANALOG_CENTER;
                            port->left_stick_center_y = CONTROLLER_ANALOG_CENTER;
                            port->analog_direction_bits = 0;
                        }
                    }
                    port->device_type = device_type;
                }
                port->held_buttons = held_buttons;

                /* Fast-repeat mode halves both the initial delay and interval. */
                if (controller_state->fast_button_repeat != 0)
                {
                    initial_repeat_delay = CONTROLLER_FAST_REPEAT_DELAY;
                    repeat_interval = CONTROLLER_FAST_REPEAT_INTERVAL;
                }
                else
                {
                    initial_repeat_delay = CONTROLLER_NORMAL_REPEAT_DELAY;
                    repeat_interval = CONTROLLER_NORMAL_REPEAT_INTERVAL;
                }
                repeat_timer_step = 1;
                if (held_buttons & PADRup)
                {
                    if ((port->pressed_buttons & PADRup) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_up = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_up - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->repeat_buttons |= PADRup;
                        }
                        port->face_repeat_timer_up = delta;
                    }
                }
                if (held_buttons & PADRright)
                {
                    if ((port->pressed_buttons & PADRright) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_right = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_right - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->repeat_buttons |= PADRright;
                        }
                        port->face_repeat_timer_right = delta;
                    }
                }
                if (held_buttons & PADRdown)
                {
                    if ((port->pressed_buttons & PADRdown) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_down = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_down - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->repeat_buttons |= PADRdown;
                        }
                        port->face_repeat_timer_down = delta;
                    }
                }
                if (held_buttons & PADRleft)
                {
                    if ((port->pressed_buttons & PADRleft) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_left = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_left - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->repeat_buttons |= PADRleft;
                        }
                        port->face_repeat_timer_left = delta;
                    }
                }
                if (device_type != CONTROLLER_DEVICE_DIGITAL)
                {
                    /* Center, clamp, and scale raw axes into signed movement values. */
                    delta = pad_packet[4] - port->right_stick_center_x;
                    if (((u32)(delta + CONTROLLER_ANALOG_DEADZONE)) < CONTROLLER_ANALOG_DEADZONE_WIDTH)
                    {
                        delta = 0;
                    }
                    if (delta < CONTROLLER_ANALOG_MIN)
                    {
                        delta = CONTROLLER_ANALOG_MIN;
                    }
                    else if (delta >= CONTROLLER_ANALOG_CENTER)
                    {
                        delta = CONTROLLER_ANALOG_MAX;
                    }
                    shifted_delta = delta >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    if (delta < 0)
                    {
                        port->right_stick_x =
                            (delta + CONTROLLER_ANALOG_NEGATIVE_ROUNDING) >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    }
                    else
                    {
                        port->right_stick_x = shifted_delta;
                    }
                    delta = pad_packet[5] - port->right_stick_center_y;
                    if (((u32)(delta + CONTROLLER_ANALOG_DEADZONE)) < CONTROLLER_ANALOG_DEADZONE_WIDTH)
                    {
                        delta = 0;
                    }
                    if (delta < CONTROLLER_ANALOG_MIN)
                    {
                        delta = CONTROLLER_ANALOG_MIN;
                    }
                    else if (delta >= CONTROLLER_ANALOG_CENTER)
                    {
                        delta = CONTROLLER_ANALOG_MAX;
                    }
                    unsigned_value = delta >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    if (delta < 0)
                    {
                        port->right_stick_y =
                            (delta + CONTROLLER_ANALOG_NEGATIVE_ROUNDING) >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    }
                    else
                    {
                        port->right_stick_y = unsigned_value;
                    }
                    delta = pad_packet[6] - port->left_stick_center_x;
                    if (((u32)(delta + CONTROLLER_ANALOG_DEADZONE)) < CONTROLLER_ANALOG_DEADZONE_WIDTH)
                    {
                        delta = 0;
                    }
                    if (delta < CONTROLLER_ANALOG_MIN)
                    {
                        delta = CONTROLLER_ANALOG_MIN;
                    }
                    else if (delta >= CONTROLLER_ANALOG_CENTER)
                    {
                        delta = CONTROLLER_ANALOG_MAX;
                    }
                    shifted_delta = delta >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    if (delta < 0)
                    {
                        shifted_delta =
                            (delta + CONTROLLER_ANALOG_NEGATIVE_ROUNDING) >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    }
                    delta = shifted_delta;
                    port->left_stick_x = delta;
                    analog_directions = PADRleft;
                    if (delta >= 0)
                    {
                        if (delta > 0)
                        {
                            analog_directions = PADRright;
                        }
                        else
                        {
                            analog_directions = 0;
                        }
                    }
                    delta = pad_packet[7] - port->left_stick_center_y;
                    if (((u32)(delta + CONTROLLER_ANALOG_DEADZONE)) < CONTROLLER_ANALOG_DEADZONE_WIDTH)
                    {
                        delta = 0;
                    }
                    if (delta < CONTROLLER_ANALOG_MIN)
                    {
                        delta = CONTROLLER_ANALOG_MIN;
                    }
                    else if (delta >= CONTROLLER_ANALOG_CENTER)
                    {
                        delta = CONTROLLER_ANALOG_MAX;
                    }
                    shifted_delta = delta >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    if (delta < 0)
                    {
                        shifted_delta =
                            (delta + CONTROLLER_ANALOG_NEGATIVE_ROUNDING) >> CONTROLLER_ANALOG_SCALE_SHIFT;
                    }
                    delta = shifted_delta;
                    port->left_stick_y = delta;
                    if (delta < 0)
                    {
                        analog_directions |= PADRup;
                    }
                    else if (delta > 0)
                    {
                        analog_directions |= PADRdown;
                    }
                    delta = analog_directions & (analog_directions ^ ((port->analog_direction_bits & 0xF) * 0x10));

                    /* Merge analog direction edges with their independent repeat timers. */
                    new_analog_directions = delta;
                    decoded_state = new_analog_directions | ((analog_directions & 0xFF) >> 4);
                    if (analog_directions & PADRup)
                    {
                        if ((new_analog_directions & PADRup) && (controller_state->sample_unavailable == 0))
                        {
                            port->direction_repeat_timer_up = initial_repeat_delay;
                        }
                        else
                        {
                            delta = port->direction_repeat_timer_up - repeat_timer_step;
                            if (delta <= 0)
                            {
                                decoded_state |= PADRup;
                                delta = repeat_interval;
                            }
                            port->direction_repeat_timer_up = delta;
                        }
                    }
                    if (analog_directions & PADRright)
                    {
                        if ((new_analog_directions & PADRright) && (controller_state->sample_unavailable == 0))
                        {
                            port->direction_repeat_timer_right = initial_repeat_delay;
                        }
                        else
                        {
                            delta = port->direction_repeat_timer_right - repeat_timer_step;
                            if (delta <= 0)
                            {
                                decoded_state |= PADRright;
                                delta = repeat_interval;
                            }
                            port->direction_repeat_timer_right = delta;
                        }
                    }
                    if (analog_directions & PADRdown)
                    {
                        if ((new_analog_directions & PADRdown) && (controller_state->sample_unavailable == 0))
                        {
                            port->direction_repeat_timer_down = initial_repeat_delay;
                        }
                        else
                        {
                            delta = port->direction_repeat_timer_down - repeat_timer_step;
                            if (delta <= 0)
                            {
                                decoded_state |= PADRdown;
                                delta = repeat_interval;
                            }
                            port->direction_repeat_timer_down = delta;
                        }
                    }
                    if (analog_directions & PADRleft)
                    {
                        if ((new_analog_directions & PADRleft) && (controller_state->sample_unavailable == 0))
                        {
                            port->direction_repeat_timer_left = initial_repeat_delay;
                        }
                        else
                        {
                            delta = port->direction_repeat_timer_left - repeat_timer_step;
                            if (delta <= 0)
                            {
                                decoded_state |= PADRleft;
                                delta = repeat_interval;
                            }
                            port->direction_repeat_timer_left = delta;
                        }
                    }
                    port->analog_direction_bits = decoded_state;
                    return;
                }
            }
            else
            {
                port->device_type = CONTROLLER_DEVICE_DISCONNECTED;
                return;
            }
        }
        else
        {
            port->device_type = CONTROLLER_DEVICE_DISCONNECTED;
            return;
        }
    }
    else
    {
        clear_controller_sample((ControllerSample*)&port->device_type);
        return;
    }
}

/**
 * @brief Poll both controller ports once when LIBPAD reports a new VSync sample.
 * @see decomp.me (100%) https://decomp.me/scratch/FnYh0
 */
void controller_poll(void)
{
    s32 actuator_power_total;
    ControllerState* controller_state = CONTROLLER_STATE;

    /* Only consume data from a completed LIBPAD transaction. */
    if (PadChkVsync() != 0)
    {
        actuator_power_total = 0;
        controller_state->actuator_cycle = (controller_state->actuator_cycle + 1) & 0xF;
        poll_controller_port(&controller_state->ports[0], &actuator_power_total);
        poll_controller_port(&controller_state->ports[1], &actuator_power_total);
        controller_state->sample_unavailable = 0;
        return;
    }

    /* Prevent stale input from being mistaken for a fresh press. */
    clear_controller_sample((ControllerSample*)&controller_state->ports[0].device_type);
    clear_controller_sample((ControllerSample*)&controller_state->ports[1].device_type);
    controller_state->sample_unavailable = 1;
}

/**
 * @brief Clear the mutable fields of a 16-byte controller sample.
 * @param sample Controller sample to clear.
 * @see decomp.me (100%) https://decomp.me/scratch/TSmff
 */
void clear_controller_sample(ControllerSample* sample)
{
    sample->repeat_buttons = 0;
    sample->pressed_buttons = 0;
    sample->held_buttons = 0;
    sample->analog_direction_bits = 0;
    sample->left_stick_y = 0;
    sample->left_stick_x = 0;
    sample->right_stick_y = 0;
    sample->right_stick_x = 0;
}

/**
 * @brief Stop LIBPAD communication and restore the previous VSync callback.
 * @see decomp.me (100%) https://decomp.me/scratch/NkZqx
 */
void shutdown_controllers(void)
{
    ControllerState* controller_state = CONTROLLER_STATE;
    VSync(0);
    PadStopCom();
    VSyncCallback(controller_state->previous_vsync_callback);
    controller_state->sample_unavailable = 0;
}

/**
 * @brief Install the controller VSync callback and reset its sample counters.
 * @see decomp.me (100%) https://decomp.me/scratch/BwqlO
 */
void initialize_controller_vsync(void)
{
    ControllerState* controller_state = CONTROLLER_STATE;
    VSync(0);
    VSyncCallback(controller_vsync_callback);
    controller_state->vsync_accumulation_count = 0;
    controller_state->pending_sample_count = 0;
}

/**
 * @brief Set a future VSync accumulation boundary or restart the current phase.
 * @param interval New boundary when it exceeds the current phase; otherwise the phase is reset.
 * @see decomp.me (100%) https://decomp.me/scratch/zw8m7
 */
void set_controller_vsync_interval(u32 interval)
{
    ControllerState* controller_state = CONTROLLER_STATE;
    if (controller_state->vsync_accumulation_count >= interval)
    {
        controller_state->vsync_accumulation_count = 0;
        return;
    }
    controller_state->vsync_accumulation_interval = interval;
}

/**
 * @brief Publish the latest controller state and transfer queued VSync samples to history.
 * @see decomp.me (100%) https://decomp.me/scratch/GawXK
 */
void update_controllers(void)
{
    ControllerSample* port2_source_sample;
    u8* controller_base;
    s32 sample_index;
    s32 port1_dest_offset;
    s32 port2_dest_offset;
    s32 port1_source_offset;
    s32 port2_source_offset;
    u8 sample_count;
    s32 history_offset;
    s32 sample_loop_end;
    controller_poll();
    controller_base = (u8*)CONTROLLER_STATE;

    /* A frame with no queued snapshots starts with a clean accumulator. */
    if (g_controller_vsync_sample_count == 0)
    {
        clear_controller_sample(&((ControllerState*)controller_base)->ports[0].accumulated_sample);
        clear_controller_sample(&((ControllerState*)controller_base)->ports[1].accumulated_sample);
    }

    /* Publish the current frame from the accumulated and latest samples. */
    merge_latest_controller_sample(&((ControllerState*)controller_base)->ports[0]);
    merge_latest_controller_sample(&((ControllerState*)controller_base)->ports[1]);
    sample_index = ((ControllerState*)controller_base)->pending_sample_count;
    sample_count = sample_index;
    ((ControllerState*)controller_base)->published_sample_count = sample_count;
    sample_index = sample_count - 1;
    if (sample_index != (-1))
    {
        sample_loop_end = -1;
        /* Publish pending VSync samples newest-first into each port's frame history. */
        history_offset = sample_index * 0x10;
        port2_source_offset = history_offset + 0x10E;
        port1_source_offset = history_offset + 0x60;
        port2_dest_offset = 0xDE;
        port1_dest_offset = 0x30;
        do
        {
            port2_source_sample = (ControllerSample*)(controller_base + port2_source_offset);
            copy_controller_sample((ControllerSample*)(controller_base + port1_source_offset),
                                   (ControllerSample*)(controller_base + port1_dest_offset));
            copy_controller_sample(port2_source_sample, (ControllerSample*)(controller_base + port2_dest_offset));
            port2_dest_offset += 0x10;
            port1_dest_offset += 0x10;
            port2_source_offset -= 0x10;
            sample_index -= 1;
            port1_source_offset -= 0x10;
        } while (sample_index != sample_loop_end);
    }

    /* Close the history batch and open a fresh accumulation window. */
    ((ControllerState*)controller_base)->pending_sample_count = 0;
    ((ControllerState*)controller_base)->vsync_accumulation_count = 1;
    ((ControllerState*)controller_base)->vsync_accumulation_interval = 0;
}

/**
 * @brief Reset controller VSync accumulation without stopping LIBPAD communication.
 * @see decomp.me (100%) https://decomp.me/scratch/RJyNE
 */
void reset_controller_vsync_state(void)
{
    ControllerState* controller_state = CONTROLLER_STATE;

    controller_state->vsync_accumulation_count = 0;
    controller_state->published_sample_count = 0;
    controller_state->pending_sample_count = 0;
    controller_state->sample_unavailable = 0;
}

/**
 * @brief Poll and accumulate controller samples from the installed VSync callback.
 * @see decomp.me (100%) https://decomp.me/scratch/srP3p
 */
void controller_vsync_callback(void)
{
    ControllerState* controller_state = CONTROLLER_STATE;

    if (g_controller_vsync_counter != 0)
    {
        controller_poll();

        /* Retain a bounded history of processed VSync snapshots for the next frame. */
        if (controller_state->pending_sample_count < CONTROLLER_MAX_PENDING_SAMPLES)
        {
            copy_controller_sample((ControllerSample*)&controller_state->ports[0].device_type,
                                   &controller_state->ports[0].vsync_samples[controller_state->pending_sample_count]);
            copy_controller_sample((ControllerSample*)&controller_state->ports[1].device_type,
                                   &controller_state->ports[1].vsync_samples[controller_state->pending_sample_count]);
            controller_state->pending_sample_count = controller_state->pending_sample_count + 1;
        }

        /* The first sample in a window replaces the previous aggregate. */
        if (controller_state->vsync_accumulation_count == 1)
        {
            clear_controller_sample(&controller_state->ports[0].accumulated_sample);
            clear_controller_sample(&controller_state->ports[1].accumulated_sample);
        }

        /* Subsequent VSyncs OR buttons and sum analog movement into the window. */
        accumulate_controller_sample(&controller_state->ports[0]);
        accumulate_controller_sample(&controller_state->ports[1]);

        {
            u8 next_accumulation_count = controller_state->vsync_accumulation_count + 1;
            u8 accumulation_interval = controller_state->vsync_accumulation_interval;
            controller_state->vsync_accumulation_count = next_accumulation_count;
            if (accumulation_interval != 0 && next_accumulation_count >= accumulation_interval)
            {
                controller_state->vsync_accumulation_count = 0;
            }
        }
    }
}

/**
 * @brief Accumulate the latest sample into a port's interval aggregate.
 * @param port Per-port controller state whose latest sample is accumulated.
 * @see decomp.me (100%) https://decomp.me/scratch/aj1vL
 */
void accumulate_controller_sample(ControllerPortState* port)
{
    s32 device_type = (s32)port->device_type;
    if (device_type == 0)
    {
    }
    else if (device_type < 0)
    {
        return;
    }
    else if (device_type >= 3)
    {
        return;
    }
    else
    {
        port->accumulated_sample.right_stick_x += port->right_stick_x;
        port->accumulated_sample.right_stick_y += port->right_stick_y;
        port->accumulated_sample.left_stick_x += port->left_stick_x;
        port->accumulated_sample.left_stick_y += port->left_stick_y;
        port->accumulated_sample.analog_direction_bits |= port->analog_direction_bits;
    }
    port->accumulated_sample.held_buttons |= port->held_buttons;
    port->accumulated_sample.pressed_buttons |= port->pressed_buttons;
    port->accumulated_sample.repeat_buttons |= port->repeat_buttons;
}

/**
 * @brief Merge the accumulated and latest samples into a port's published sample.
 * @param port Per-port controller state to publish.
 * @see decomp.me (100%) https://decomp.me/scratch/ORRFA
 */
void merge_latest_controller_sample(ControllerPortState* port)
{
    s32 device_type = port->device_type;
    port->published_sample.device_type = device_type;
    device_type &= 0xFF;

    if (device_type == 0)
    {
        /* Digital controllers publish buttons but no analog fields. */
    }
    else
    {
        if (device_type < 0)
        {
            return;
        }
        if (device_type >= 3)
        {
            return;
        }
        port->published_sample.right_stick_x = port->accumulated_sample.right_stick_x + port->right_stick_x;
        port->published_sample.right_stick_y = port->accumulated_sample.right_stick_y + port->right_stick_y;
        port->published_sample.left_stick_x = port->accumulated_sample.left_stick_x + port->left_stick_x;
        port->published_sample.left_stick_y = port->accumulated_sample.left_stick_y + port->left_stick_y;
        port->published_sample.analog_direction_bits =
            port->accumulated_sample.analog_direction_bits | port->analog_direction_bits;
    }

    port->published_sample.held_buttons = port->accumulated_sample.held_buttons | port->held_buttons;
    port->published_sample.pressed_buttons = port->accumulated_sample.pressed_buttons | port->pressed_buttons;
    port->published_sample.repeat_buttons = port->accumulated_sample.repeat_buttons | port->repeat_buttons;
}

/**
 * @brief Copy one 16-byte controller sample while preserving device-type handling.
 * @param source Controller sample to copy.
 * @param destination Controller sample to overwrite.
 * @see decomp.me (100%) https://decomp.me/scratch/Hkz5t
 */
void copy_controller_sample(ControllerSample* source, ControllerSample* destination)
{
    s32 device_type;

    clear_controller_sample(destination);
    destination->device_type = source->device_type;
    device_type = (s32)source->device_type;

    if (device_type == 0)
    {
        /* Digital controllers copy buttons but no analog fields. */
    }
    else
    {
        if (device_type < 0)
        {
            return;
        }
        if (device_type >= 3)
        {
            return;
        }
        destination->right_stick_x = source->right_stick_x;
        destination->right_stick_y = source->right_stick_y;
        destination->left_stick_x = source->left_stick_x;
        destination->left_stick_y = source->left_stick_y;
        destination->analog_direction_bits = source->analog_direction_bits;
    }

    /* All supported controller classes share the button fields. */
    destination->held_buttons = source->held_buttons;
    destination->pressed_buttons = source->pressed_buttons;
    destination->repeat_buttons = source->repeat_buttons;
}
