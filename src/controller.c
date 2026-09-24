#include "controller.h"
#include "sdk/libetc.h"
#include "sdk/libpad.h"
#include "controller_internal.h"

void PadStartCom();

void controller_poll(void);
void clear_controller_sample(ControllerSample* sample);

void controller_vsync_callback(void);
void accumulate_controller_sample(ControllerPortState* port);
void merge_latest_controller_sample(ControllerPortState* port);
void copy_controller_sample(ControllerSample* source, ControllerSample* destination);
void poll_controller_port(ControllerPortState* port, s32* actuator_current_total);

/**
 * @brief Initialize LIBPAD, both controller-port records, and their receive buffers.
 * @param enable_actuators Nonzero to enable actuator updates for each port.
 * @see decomp.me (100%) https://decomp.me/scratch/b48Yj
 */
void initialize_controllers(s8 enable_actuators)
{
    ControllerState* controller_state;
    ControllerPortState* current_port;
    ControllerPortState* status_port;
    u16 actuator_control;
    u16 actuator_status;
    s32 port_index;
    u32 disconnected_device_type;
    s32 legacy_vibration_device_id;
    s32 all_ports_ready;
    s32 status_index;

    PadInitDirect(CONTROLLER_STATE->receive_buffers[0].bytes, CONTROLLER_STATE->receive_buffers[1].bytes);
    g_previous_controller_vsync_callback.address = VSyncCallback(NULL);
    controller_state = CONTROLLER_STATE;

    controller_state->ports[0].port_id = 0;
    controller_state->ports[1].port_id = CONTROLLER_SECOND_PORT_FLAG;
    clear_controller_sample(&controller_state->ports[0].published_sample);
    clear_controller_sample(&controller_state->ports[1].published_sample);
    clear_controller_sample(&controller_state->ports[0].current_sample);
    clear_controller_sample(&controller_state->ports[1].current_sample);
    port_index = CONTROLLER_PORT_COUNT - 1;
    legacy_vibration_device_id = CONTROLLER_LEGACY_VIBRATION_DEVICE_ID;
    disconnected_device_type = CONTROLLER_DEVICE_DISCONNECTED;

    for (; port_index != -1; port_index--)
    {
        current_port = &controller_state->ports[port_index];
        actuator_control = current_port->actuator_control.value;
        current_port->legacy_vibration_device_id = legacy_vibration_device_id;
        current_port->actuator_values[2] = 0;
        current_port->actuator_values[1] = 0;
        current_port->actuator_values[0] = 0;
        current_port->actuators_enabled = enable_actuators;
        current_port->small_motor_command = 0;
        current_port->actuator_count = 0;
        current_port->small_motor_current = 0;
        current_port->large_motor_current = 0;
        current_port->current_sample.device_type = disconnected_device_type;
        current_port->published_sample.device_type = disconnected_device_type;
        actuator_control &= ~CONTROLLER_ACTUATOR_RUNTIME_FLAGS_MASK;
        current_port->actuator_control.value = actuator_control;
        current_port->actuator_control.fields.large_motor_command = 0;
    }

    controller_state->fast_button_repeat = 0;
    controller_state->actuator_cycle = 0;
    controller_state->published_sample_count = 0;
    controller_state->pending_sample_count = 0;
    controller_state->sample_unavailable = 0;

    /* PadStartCom takes no arguments; the original call site leaves the device type in $a0. */
    PadStartCom(disconnected_device_type);
    do
    {
        VSync(0);
        controller_poll();
        all_ports_ready = 1;
        for (status_index = CONTROLLER_PORT_COUNT - 1; status_index != -1; status_index--)
        {
            status_port = &controller_state->ports[status_index];
            actuator_status = status_port->actuator_control.value;
            if ((!CONTROLLER_IS_DISCONNECTED(actuator_status)) && (CONTROLLER_ACTUATOR_SETUP_STATE(actuator_status) != CONTROLLER_ACTUATOR_SETUP_READY))
            {
                all_ports_ready = 0;
            }
        }
    } while (all_ports_ready == 0);

    controller_state->vsync_accumulation_count = 0;
    controller_state->vsync_accumulation_interval = 0;
}

/**
 * @brief Poll one LIBPAD port and update buttons, analog axes, repeat state, and actuators.
 * @param port Per-port controller state to update.
 * @param actuator_current_total Accumulator for the active actuators' current draw.
 * @see decomp.me (100%) https://decomp.me/scratch/rDO0T
 */
void poll_controller_port(ControllerPortState* port, s32* actuator_current_total)
{
    s32 shifted_delta;
    s32 repeat_timer_step;
    ControllerState* controller_state;
    u32 pad_state;
    s32 counter;
    u32 updated_actuator_config;
    s32 mode_index;
    u32 unsigned_value;
    s32 remaining_actuators;
    s32 multitap_slot;
    u8 device_type;
    s32 decoded_state;
    u8 initial_repeat_delay;
    s32 repeat_interval;
    u16 held_buttons;
    s32 delta;
    s32 new_analog_directions;
    u32 analog_directions;
    ControllerPacketView packet;
    controller_state = CONTROLLER_STATE;

    pad_state = PadGetState(port->port_id);
    switch (pad_state)
    {
    case PadStateDiscon:
        updated_actuator_config = port->actuator_control.value | CONTROLLER_DISCONNECTED_FLAG;
        port->current_sample.device_type = CONTROLLER_DEVICE_DISCONNECTED;
        port->actuator_control.value = updated_actuator_config & ~CONTROLLER_ACTUATOR_SETUP_MASK;
        return;

    case PadStateFindPad:
        port->actuator_control.value &= ~CONTROLLER_DISCONNECTED_FLAG;
        if (port->actuator_control.value & CONTROLLER_ACTUATOR_SETUP_MASK)
        {
            port->actuator_control.value =
                (port->actuator_control.value & ~(CONTROLLER_DISCONNECTED_FLAG | CONTROLLER_ACTUATOR_SETUP_MASK)) | CONTROLLER_ACTUATOR_SETUP_MODE;
        }
        /* Fall through while LIBPAD is negotiating the controller. */

    case PadStateReqInfo:
    case PadStateExecCmd:
        port->current_sample.device_type = CONTROLLER_DEVICE_CONFIGURING;
        clear_controller_sample(&port->current_sample);
        return;

    case PadStateFindCTP1:
        port->legacy_vibration_device_id = CONTROLLER_LEGACY_VIBRATION_DEVICE_ID;
        if (port->actuators_enabled != 0)
        {
            if (port->small_motor_command & CONTROLLER_SMALL_MOTOR_ENABLE_FLAG)
            {
                port->actuator_values[0] = port->small_motor_command;
                *actuator_current_total += port->small_motor_current;
            }
            else
            {
                u8 large_motor_command = port->actuator_control.fields.large_motor_command;
                if ((large_motor_command != 0) && ((large_motor_command * CONTROLLER_ACTUATOR_CYCLE_STEPS) >= controller_state->actuator_cycle))
                {
                    port->actuator_values[0] = CONTROLLER_SMALL_MOTOR_ENABLE_FLAG;
                    *actuator_current_total += port->small_motor_current;
                }
                else
                {
                    port->actuator_values[0] = 0;
                }
            }
        }
        else
        {
            port->actuator_values[0] = 0;
        }

        if ((port->actuator_control.value & CONTROLLER_ACTUATOR_SETUP_MASK) != CONTROLLER_ACTUATOR_SETUP_ACTIVE)
        {
            PadSetAct(port->port_id, &port->legacy_vibration_device_id, 2);
            port->small_motor_current = CONTROLLER_LEGACY_ACTUATOR_CURRENT;
            port->actuator_control.value =
                (port->actuator_control.value | CONTROLLER_ACTUATOR_SETUP_ACTIVE) & ~(CONTROLLER_ACTUATOR_SETUP_MODE | CONTROLLER_USE_DEFAULT_ANALOG_CENTER);
        }
        break;

    case PadStateStable:
        if (port->actuators_enabled != 0)
        {
            u8 large_motor_command = port->actuator_control.fields.large_motor_command;
            if (large_motor_command != 0)
            {
                port->actuator_values[1] = large_motor_command;
                *actuator_current_total += port->large_motor_current;
            }
            else
            {
                port->actuator_values[1] = 0;
            }
            if (port->small_motor_command & CONTROLLER_SMALL_MOTOR_ENABLE_FLAG)
            {
                port->actuator_values[0] = CONTROLLER_SMALL_MOTOR_ENABLE_FLAG;
                *actuator_current_total += port->small_motor_current;
            }
            else
            {
                port->actuator_values[0] = 0;
            }
        }
        else
        {
            port->actuator_values[0] = 0;
            port->actuator_values[1] = 0;
        }

        switch (CONTROLLER_ACTUATOR_SETUP_STATE(port->actuator_control.value))
        {
        case 0:
            port->actuator_control.value = (port->actuator_control.value & ~CONTROLLER_ACTUATOR_SETUP_MASK) | CONTROLLER_ACTUATOR_SETUP_MODE;

            counter = PadInfoMode(port->port_id, InfoModeIdTable, -1);
            mode_index = 0;
            unsigned_value = counter;
            if (unsigned_value != 0)
            {
                counter--;
                do
                {
                    if (PadInfoMode(port->port_id, InfoModeIdTable, mode_index) != CONTROLLER_PACKET_DUALSHOCK)
                    {
                        mode_index++;
                        counter--;
                        continue;
                    }
                    if (PadInfoMode(port->port_id, InfoModeCurExOffs, 0) != mode_index)
                    {
                        PadSetMainMode(port->port_id, mode_index, PadModeUnlock);
                        port->current_sample.device_type = CONTROLLER_DEVICE_CONFIGURING;
                        clear_controller_sample(&port->current_sample);
                        return;
                    }
                    mode_index++;
                    counter--;
                } while (counter != -1);
            }
            /* Fall through to discover and align the controller's actuators. */

        case 1:
            port->actuator_control.value =
                (port->actuator_control.value & ~CONTROLLER_ACTUATOR_SETUP_MASK) | CONTROLLER_ACTUATOR_SETUP_ACTIVE | CONTROLLER_USE_DEFAULT_ANALOG_CENTER;
            counter = CONTROLLER_ACTUATOR_ALIGNMENT_COUNT - 1;
            while (counter != -1)
            {
                port->actuator_alignment[counter] = CONTROLLER_ACTUATOR_UNMAPPED;
                counter--;
            }

            mode_index = 0;
            remaining_actuators = PadInfoAct(port->port_id, -1, mode_index);
            port->actuator_count = remaining_actuators;
            PadSetAct(port->port_id, &port->actuator_values[0], remaining_actuators);
            counter = mode_index;
            remaining_actuators--;
            port->small_motor_current = 0;
            port->large_motor_current = 0;
            while (remaining_actuators != -1)
            {
                s32 actuator_supported = PadInfoAct(port->port_id, counter, InfoActFunc);
                if (actuator_supported == 1)
                {
                    switch (PadInfoAct(port->port_id, counter, InfoActSize))
                    {
                    case 0:
                        if (port->actuator_alignment[0] == CONTROLLER_ACTUATOR_UNMAPPED)
                        {
                            port->small_motor_current = PadInfoAct(port->port_id, counter, InfoActCurr);
                            port->actuator_alignment[0] = counter;
                        }
                        break;
                    case 1:
                        if (port->actuator_alignment[1] == CONTROLLER_ACTUATOR_UNMAPPED)
                        {
                            port->large_motor_current = PadInfoAct(port->port_id, counter, InfoActCurr);
                            port->actuator_alignment[1] = counter;
                        }
                        break;
                    }
                }
                counter++;
                remaining_actuators--;
            }

            PadSetActAlign(port->port_id, port->actuator_alignment);
            port->current_sample.device_type = CONTROLLER_DEVICE_CONFIGURING;
            clear_controller_sample(&port->current_sample);
            return;

        default:
            break;
        }

        break;

    default:
        break;
    }

    if (port->port_id & CONTROLLER_SECOND_PORT_FLAG)
    {
        packet.receive = &controller_state->receive_buffers[1];
    }
    else
    {
        packet.receive = &controller_state->receive_buffers[0];
    }
    multitap_slot = port->port_id & CONTROLLER_MULTITAP_SLOT_MASK;
    if (multitap_slot != 0)
    {
        if (packet.receive->packet_id == CONTROLLER_MULTITAP_PACKET_ID)
        {
            packet.pad = &packet.receive->multitap.slots[multitap_slot];
        }
        else
        {
            port->current_sample.device_type = CONTROLLER_DEVICE_DISCONNECTED;
            return;
        }
    }
    else if (packet.receive->packet_id == CONTROLLER_MULTITAP_PACKET_ID)
    {
        packet.pad = &packet.receive->multitap.slots[0];
    }

    if (packet.pad->status == 0)
    {
        u8 controller_id = packet.pad->id;
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
        if (decoded_state < CONTROLLER_SUPPORTED_DEVICE_TYPE_COUNT)
        {
            if (decoded_state >= 0)
            {
                held_buttons = ~packet.pad->buttons;
                if (port->current_sample.device_type == decoded_state)
                {
                    port->current_sample.pressed_buttons =
                        (port->current_sample.repeat_buttons = held_buttons & (port->current_sample.held_buttons ^ held_buttons));
                }
                else
                {
                    port->current_sample.pressed_buttons = (port->current_sample.repeat_buttons = held_buttons);
                    if (decoded_state == CONTROLLER_DEVICE_ANALOG_JOYSTICK)
                    {
                        port->right_stick_center_x = packet.pad->right_stick_x;
                        port->right_stick_center_y = packet.pad->right_stick_y;
                        port->left_stick_center_x = packet.pad->left_stick_x;
                        port->left_stick_center_y = packet.pad->left_stick_y;
                        port->current_sample.analog_direction_bits = 0;
                    }
                    else if (decoded_state == CONTROLLER_DEVICE_ANALOG)
                    {
                        if (!(port->actuator_control.value & CONTROLLER_USE_DEFAULT_ANALOG_CENTER))
                        {
                            port->right_stick_center_x = packet.pad->right_stick_x;
                            port->right_stick_center_y = packet.pad->right_stick_y;
                            port->left_stick_center_x = packet.pad->left_stick_x;
                            port->left_stick_center_y = packet.pad->left_stick_y;
                            port->current_sample.analog_direction_bits = 0;
                        }
                        else
                        {
                            port->right_stick_center_x = CONTROLLER_ANALOG_CENTER;
                            port->right_stick_center_y = CONTROLLER_ANALOG_CENTER;
                            port->left_stick_center_x = CONTROLLER_ANALOG_CENTER;
                            port->left_stick_center_y = CONTROLLER_ANALOG_CENTER;
                            port->current_sample.analog_direction_bits = 0;
                        }
                    }
                    port->current_sample.device_type = device_type;
                }
                port->current_sample.held_buttons = held_buttons;

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
                    if ((port->current_sample.pressed_buttons & PADRup) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_up = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_up - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->current_sample.repeat_buttons |= PADRup;
                        }
                        port->face_repeat_timer_up = delta;
                    }
                }
                if (held_buttons & PADRright)
                {
                    if ((port->current_sample.pressed_buttons & PADRright) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_right = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_right - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->current_sample.repeat_buttons |= PADRright;
                        }
                        port->face_repeat_timer_right = delta;
                    }
                }
                if (held_buttons & PADRdown)
                {
                    if ((port->current_sample.pressed_buttons & PADRdown) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_down = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_down - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->current_sample.repeat_buttons |= PADRdown;
                        }
                        port->face_repeat_timer_down = delta;
                    }
                }
                if (held_buttons & PADRleft)
                {
                    if ((port->current_sample.pressed_buttons & PADRleft) && (controller_state->sample_unavailable == 0))
                    {
                        port->face_repeat_timer_left = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->face_repeat_timer_left - repeat_timer_step;
                        if (delta <= 0)
                        {
                            delta = repeat_interval;
                            port->current_sample.repeat_buttons |= PADRleft;
                        }
                        port->face_repeat_timer_left = delta;
                    }
                }
                if (device_type != CONTROLLER_DEVICE_DIGITAL)
                {
                    delta = packet.pad->right_stick_x - port->right_stick_center_x;
                    if (CONTROLLER_IS_WITHIN_ANALOG_DEADZONE(delta))
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
                        port->current_sample.right_stick_x = delta / (1 << CONTROLLER_ANALOG_SCALE_SHIFT);
                    }
                    else
                    {
                        port->current_sample.right_stick_x = shifted_delta;
                    }
                    delta = packet.pad->right_stick_y - port->right_stick_center_y;
                    if (CONTROLLER_IS_WITHIN_ANALOG_DEADZONE(delta))
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
                        port->current_sample.right_stick_y = delta / (1 << CONTROLLER_ANALOG_SCALE_SHIFT);
                    }
                    else
                    {
                        port->current_sample.right_stick_y = unsigned_value;
                    }

                    delta = packet.pad->left_stick_x - port->left_stick_center_x;
                    if (CONTROLLER_IS_WITHIN_ANALOG_DEADZONE(delta))
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
                        shifted_delta = delta / (1 << CONTROLLER_ANALOG_SCALE_SHIFT);
                    }
                    delta = shifted_delta;
                    port->current_sample.left_stick_x = delta;
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
                    delta = packet.pad->left_stick_y - port->left_stick_center_y;
                    if (CONTROLLER_IS_WITHIN_ANALOG_DEADZONE(delta))
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
                        shifted_delta = delta / (1 << CONTROLLER_ANALOG_SCALE_SHIFT);
                    }
                    delta = shifted_delta;
                    port->current_sample.left_stick_y = delta;
                    if (delta < 0)
                    {
                        analog_directions |= PADRup;
                    }
                    else if (delta > 0)
                    {
                        analog_directions |= PADRdown;
                    }

                    delta = analog_directions & (analog_directions ^ ((port->current_sample.analog_direction_bits & CONTROLLER_ANALOG_DIRECTION_HELD_MASK)
                                                                      << CONTROLLER_ANALOG_DIRECTION_EVENT_SHIFT));

                    new_analog_directions = delta;
                    decoded_state = new_analog_directions | ((u8)analog_directions >> CONTROLLER_ANALOG_DIRECTION_EVENT_SHIFT);
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
                    port->current_sample.analog_direction_bits = decoded_state;
                    return;
                }
            }
            else
            {
                port->current_sample.device_type = CONTROLLER_DEVICE_DISCONNECTED;
                return;
            }
        }
        else
        {
            port->current_sample.device_type = CONTROLLER_DEVICE_DISCONNECTED;
            return;
        }
    }
    else
    {
        clear_controller_sample(&port->current_sample);
        return;
    }
}

/**
 * @brief Poll both controller ports once when LIBPAD reports a new VSync sample.
 * @see decomp.me (100%) https://decomp.me/scratch/FnYh0
 */
void controller_poll(void)
{
    s32 actuator_current_total;
    ControllerState* controller_state = CONTROLLER_STATE;

    if (PadChkVsync() != 0)
    {
        actuator_current_total = 0;
        controller_state->actuator_cycle = (controller_state->actuator_cycle + 1) & CONTROLLER_ACTUATOR_CYCLE_MASK;
        poll_controller_port(&controller_state->ports[0], &actuator_current_total);
        poll_controller_port(&controller_state->ports[1], &actuator_current_total);
        controller_state->sample_unavailable = 0;
        return;
    }

    clear_controller_sample(&controller_state->ports[0].current_sample);
    clear_controller_sample(&controller_state->ports[1].current_sample);
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
    VSyncCallback(controller_state->previous_vsync_callback.handler);
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
    ControllerState* controller_state;
    s32 sample_index;
    s32 history_index;
    ControllerSample* first_source;
    ControllerSample* second_source;
    u8 sample_count;
    controller_poll();
    controller_state = CONTROLLER_STATE;

    if (g_controller_vsync_sample_count == 0)
    {
        clear_controller_sample(&controller_state->ports[0].accumulated_sample);
        clear_controller_sample(&controller_state->ports[1].accumulated_sample);
    }

    merge_latest_controller_sample(&controller_state->ports[0]);
    merge_latest_controller_sample(&controller_state->ports[1]);
    sample_index = controller_state->pending_sample_count;
    sample_count = sample_index;
    controller_state->published_sample_count = sample_count;
    sample_index = sample_count - 1;
    for (history_index = 0; sample_index != -1; history_index++, sample_index--)
    {
        first_source = &controller_state->ports[0].vsync_samples[sample_index];
        second_source = &controller_state->ports[1].vsync_samples[sample_index];
        copy_controller_sample(first_source, &controller_state->ports[0].frame_samples[history_index]);
        copy_controller_sample(second_source, &controller_state->ports[1].frame_samples[history_index]);
    }

    controller_state->pending_sample_count = 0;
    controller_state->vsync_accumulation_count = 1;
    controller_state->vsync_accumulation_interval = 0;
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

        if (controller_state->pending_sample_count < CONTROLLER_MAX_PENDING_SAMPLES)
        {
            copy_controller_sample(&controller_state->ports[0].current_sample,
                                   &controller_state->ports[0].vsync_samples[controller_state->pending_sample_count]);
            copy_controller_sample(&controller_state->ports[1].current_sample,
                                   &controller_state->ports[1].vsync_samples[controller_state->pending_sample_count]);
            controller_state->pending_sample_count = controller_state->pending_sample_count + 1;
        }

        if (controller_state->vsync_accumulation_count == 1)
        {
            clear_controller_sample(&controller_state->ports[0].accumulated_sample);
            clear_controller_sample(&controller_state->ports[1].accumulated_sample);
        }

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
    s32 device_type = port->current_sample.device_type;
    switch (device_type)
    {
    case CONTROLLER_DEVICE_DIGITAL:
        break;
    case CONTROLLER_DEVICE_ANALOG_JOYSTICK:
    case CONTROLLER_DEVICE_ANALOG:
        port->accumulated_sample.right_stick_x += port->current_sample.right_stick_x;
        port->accumulated_sample.right_stick_y += port->current_sample.right_stick_y;
        port->accumulated_sample.left_stick_x += port->current_sample.left_stick_x;
        port->accumulated_sample.left_stick_y += port->current_sample.left_stick_y;
        port->accumulated_sample.analog_direction_bits |= port->current_sample.analog_direction_bits;
        break;
    default:
        return;
    }
    port->accumulated_sample.held_buttons |= port->current_sample.held_buttons;
    port->accumulated_sample.pressed_buttons |= port->current_sample.pressed_buttons;
    port->accumulated_sample.repeat_buttons |= port->current_sample.repeat_buttons;
}

/**
 * @brief Merge the accumulated and latest samples into a port's published sample.
 * @param port Per-port controller state to publish.
 * @see decomp.me (100%) https://decomp.me/scratch/ORRFA
 */
void merge_latest_controller_sample(ControllerPortState* port)
{
    s32 device_type = port->current_sample.device_type;
    port->published_sample.device_type = device_type;
    device_type = port->published_sample.device_type;

    switch (device_type)
    {
    case CONTROLLER_DEVICE_DIGITAL:
        break;
    case CONTROLLER_DEVICE_ANALOG_JOYSTICK:
    case CONTROLLER_DEVICE_ANALOG:
        port->published_sample.right_stick_x = port->accumulated_sample.right_stick_x + port->current_sample.right_stick_x;
        port->published_sample.right_stick_y = port->accumulated_sample.right_stick_y + port->current_sample.right_stick_y;
        port->published_sample.left_stick_x = port->accumulated_sample.left_stick_x + port->current_sample.left_stick_x;
        port->published_sample.left_stick_y = port->accumulated_sample.left_stick_y + port->current_sample.left_stick_y;
        port->published_sample.analog_direction_bits = port->accumulated_sample.analog_direction_bits | port->current_sample.analog_direction_bits;
        break;
    default:
        return;
    }

    port->published_sample.held_buttons = port->accumulated_sample.held_buttons | port->current_sample.held_buttons;
    port->published_sample.pressed_buttons = port->accumulated_sample.pressed_buttons | port->current_sample.pressed_buttons;
    port->published_sample.repeat_buttons = port->accumulated_sample.repeat_buttons | port->current_sample.repeat_buttons;
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
    device_type = source->device_type;

    switch (device_type)
    {
    case CONTROLLER_DEVICE_DIGITAL:
        break;
    case CONTROLLER_DEVICE_ANALOG_JOYSTICK:
    case CONTROLLER_DEVICE_ANALOG:
        destination->right_stick_x = source->right_stick_x;
        destination->right_stick_y = source->right_stick_y;
        destination->left_stick_x = source->left_stick_x;
        destination->left_stick_y = source->left_stick_y;
        destination->analog_direction_bits = source->analog_direction_bits;
        break;
    default:
        return;
    }

    destination->held_buttons = source->held_buttons;
    destination->pressed_buttons = source->pressed_buttons;
    destination->repeat_buttons = source->repeat_buttons;
}
