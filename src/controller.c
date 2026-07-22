#include "controller.h"

/**
 * @brief Initialize LIBPAD, both controller-port records, and their receive buffers.
 * @param enable_actuators Nonzero to enable actuator updates for each port.
 * @see decomp.me (100%) https://decomp.me/scratch/b48Yj
 */
void initialize_controllers(s8 enable_actuators)
{
    u8* controller_base;
    u8** port_pointer;
    u8* port_bytes;
    u8* status_port;
    u8* second_port_sample;
    int loop_condition;
    u16 port_config;
    u16 port_status;
    int port_index;
    int loop_end;
    unsigned int invalid_device_type;
    int initial_actuator_command;
    int all_ports_ready;
    int ports_remaining;
    PadInitDirect((void*)0x801ED75C, (void*)0x801ED77E);
    g_previous_controller_vsync_callback = VSyncCallback(0);
    controller_base = (u8*)0x801ED600;
    controller_base[0xAD] = 0;
    controller_base[0x15B] = 0x10;
    clear_controller_sample(controller_base);
    clear_controller_sample(controller_base + 0xAE);
    clear_controller_sample(controller_base + 0x20);
    second_port_sample = controller_base + 0xCE;
    clear_controller_sample(second_port_sample);
    port_index = 1;
    initial_actuator_command = 0x40;
    invalid_device_type = 0xFF;
    loop_end = -1;
    port_bytes = controller_base + 0xAE;
    do
    {
        port_config = *((u16*)((*(port_pointer = &port_bytes)) + 0x92));
        port_index--;
        port_bytes[0x94] = initial_actuator_command;
        port_bytes[0x97] = 0;
        port_bytes[0x96] = 0;
        port_bytes[0x95] = 0;
        port_bytes[0x90] = enable_actuators;
        port_bytes[0x91] = 0;
        port_bytes[0xAA] = 0;
        port_bytes[0xAB] = 0;
        port_bytes[0xAC] = 0;
        port_bytes[0x20] = invalid_device_type;
        port_bytes[0x00] = invalid_device_type;
        port_config &= 0xF0FF;
        *((u16*)(port_bytes - -0x92)) = port_config;
        port_bytes[0x92] = 0;
        port_bytes -= 0xAE;
    } while (loop_condition = port_index != loop_end);
    controller_base[0x1A8] = 0 * 0;
    controller_base[0x1A9] = 0;
    controller_base[0x1A0] = 0;
    controller_base[0x1A1] = 0;
    controller_base[0x1AA] = 0;
    PadStartCom(invalid_device_type, port_index, loop_end, initial_actuator_command);
    do
    {
        VSync(0);
        controller_poll();
        all_ports_ready = 1;
        ports_remaining = all_ports_ready;
        status_port = controller_base + 0xAE;
        do
        {
            port_status = *((u16*)((*(port_pointer = &status_port)) + 0x92));
            if ((!(((port_status >> 6) >> 2) & 1)) && (((port_status >> 9) & 3) != 2))
            {
                all_ports_ready = 0;
            }
            ports_remaining--;
            status_port -= 0xAE;
        } while (ports_remaining != (-1));
    } while (all_ports_ready == 0);
    controller_base[0x1A2] = 0;
    controller_base[0x1A3] = 0;
}

/**
 * @brief Poll one LIBPAD port and update buttons, analog axes, repeat state, and actuators.
 * @param port Per-port controller state to update.
 * @param actuator_power_total Accumulator for the active actuators' current draw.
 * @see decomp.me (100%) https://decomp.me/scratch/rDO0T
 */
void poll_controller_port(ControllerPortState* port, s32* actuator_power_total)
{
    int one;
    u8* controller_base;
    u32 pad_state;
    s32 count;
    int updated_flags;
    s32 index;
    unsigned int unsigned_temp;
    s32 remaining;
    s32 shifted_delta;
    s32 loop_end;
    int slot;
    u8 disabled_actuator;
    s32 dualshock_mode_id;
    s32 fill_end;
    u8 device_type;
    int direction_bits;
    u8 initial_repeat_delay;
    s32 repeat_interval;
    u16 held_buttons;
    s32 delta;
    s32 new_direction_bits;
    u32 direction_mask;
    int detected_actuator_count;
    u8* raw_pad_data;
    controller_base = (u8*)0x801ED600;
    pad_state = PadGetState(port->port_id);
    switch (pad_state)
    {
    case 0:
        updated_flags = port->actuator_config | 0x100;
        port->device_type = 0xFF;
        port->actuator_config = updated_flags & 0xF9FF;
        return;

    case 1:
        port->actuator_config &= 0xFEFF;
        if (port->actuator_config & 0x600)
        {
            port->actuator_config = (port->actuator_config & 0xF8FF) | 0x200;
        }

    case 4:

    case 5:
        port->device_type = 0xFE;
        clear_controller_sample(((u8*)port) + 0x20);
        return;

    case 2:
        port->actuator_header = 0x40;
        if (port->actuators_enabled != 0)
        {
            if (port->small_motor_command & 1)
            {
                port->small_motor_value = port->small_motor_command;
                *actuator_power_total += port->small_motor_power;
            }
            else
            {
                u8 large_motor_command = port->actuator_config;
                if ((large_motor_command != 0) && ((large_motor_command * 0x10) >= controller_base[0x1A9]))
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
        if ((port->actuator_config & 0x600) != 0x400)
        {
            PadSetAct(port->port_id, &port->actuator_header, 2);
            port->small_motor_power = 10;
            port->actuator_config = (port->actuator_config | 0x400) & 0xF5FF;
        }
        break;

    case 6:
        if (port->actuators_enabled != 0)
        {
            u8 large_motor_command = port->actuator_config;
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
        switch ((port->actuator_config >> 9) & 3)
        {
        case 0:
            port->actuator_config = (port->actuator_config & 0xF9FF) | 0x200;
            
            count = PadInfoMode(port->port_id, 4, -1);
            index = 0;
            unsigned_temp = count;
            if (unsigned_temp != 0)
            {
                count--;
                dualshock_mode_id = 7;
                loop_end = -1;
                do
                {
                    if (PadInfoMode(port->port_id, 4, index) != dualshock_mode_id)
                    {
                        index++;
                        count--;
                        continue;
                    }
                    if (PadInfoMode(port->port_id, 3, 0) != index)
                    {
                        PadSetMainMode(port->port_id, index, 0);
                        port->device_type = 0xFE;
                        clear_controller_sample(((u8*)port) + 0x20);
                        return;
                    }
                    index++;
                    count--;
                } while (count != loop_end);
            }

        case 1:
            port->actuator_config = (port->actuator_config & 0xF9FF) | 0xC00;
            count = 5;
            disabled_actuator = 0xFF;
            fill_end = -1;
            while (count != fill_end)
            {
                ((u8*)port + count)[0x98] = disabled_actuator;
                count--;
            }

            index++;
            remaining = PadInfoAct(port->port_id, -1, index = 0);
            detected_actuator_count = remaining;
            port->actuator_count = detected_actuator_count;
            PadSetAct(port->port_id, &port->small_motor_value, remaining);
            count = index;
            remaining--;
            port->small_motor_power = 0;
            port->large_motor_power = 0;
            while (remaining != (-1))
            {
                int actuator_supported = PadInfoAct(port->port_id, count, 1);
                if (actuator_supported == 1)
                {
                    switch (PadInfoAct(port->port_id, count, 3))
                    {
                    case 0:
                        if (port->small_motor_index == 0xFF)
                        {
                            port->small_motor_power = PadInfoAct(port->port_id, count, 4);
                            port->small_motor_index = count;
                        }
                        break;
                    case 1:
                        if (port->large_motor_index == 0xFF)
                        {
                            port->large_motor_power = PadInfoAct(port->port_id, count, 4);
                            port->large_motor_index = count;
                        }
                        break;
                    }
                }
                count++;
                remaining--;
            }

            PadSetActAlign(port->port_id, &port->small_motor_index);
            port->device_type = 0xFE;
            clear_controller_sample(((u8*)port) + 0x20);
            return;

        default:
            break;
        }

        break;

    default:
        break;
    }

    if (port->port_id & 0x10)
    {
        raw_pad_data = controller_base + 0x17E;
    }
    else
    {
        raw_pad_data = controller_base + 0x15C;
    }
    slot = port->port_id & 0xF;
    if (slot != 0)
    {
        if ((*((u16*)raw_pad_data)) == 0x8000)
        {
            raw_pad_data += (slot * 8) + 2;
        }
        else
        {
            port->device_type = 0xFF;
            return;
        }
    }
    else if ((*((u16*)raw_pad_data)) == 0x8000)
    {
        raw_pad_data += 2;
    }
    if (raw_pad_data[0] == 0)
    {
        u8 controller_id = raw_pad_data[1];
        u8 controller_class = controller_id >> 4;
        switch (controller_class)
        {
        case 4:
            device_type = 0;
            break;
        case 5:
            device_type = 1;
            break;
        case 7:
            device_type = 2;
            break;
        default:
            device_type = 0xFF;
            break;
        }
        direction_bits = device_type;
        if (direction_bits < 3)
        {
            if (direction_bits >= 0)
            {
            held_buttons = ~(*((u16*)(raw_pad_data + 2)));
            if (port->device_type == direction_bits)
            {
                port->pressed_buttons = (port->repeat_buttons = held_buttons & (port->held_buttons ^ held_buttons));
            }
            else
            {
                port->pressed_buttons = (port->repeat_buttons = held_buttons);
                if (direction_bits == 1)
                {
                    port->right_stick_center_x = raw_pad_data[4];
                    port->right_stick_center_y = raw_pad_data[5];
                    port->left_stick_center_x = raw_pad_data[6];
                    port->left_stick_center_y = raw_pad_data[7];
                    port->analog_direction_bits = 0;
                }
                else if (direction_bits == 2)
                {
                    if (!(port->actuator_config & 0x800))
                    {
                        port->right_stick_center_x = raw_pad_data[4];
                        port->right_stick_center_y = raw_pad_data[5];
                        port->left_stick_center_x = raw_pad_data[6];
                        port->left_stick_center_y = raw_pad_data[7];
                        port->analog_direction_bits = 0;
                    }
                    else
                    {
                        port->right_stick_center_x = 0x80;
                        port->right_stick_center_y = 0x80;
                        port->left_stick_center_x = 0x80;
                        port->left_stick_center_y = 0x80;
                        port->analog_direction_bits = 0;
                    }
                }
                port->device_type = device_type;
            }
            port->held_buttons = held_buttons;
            if (controller_base[0x1A8] != 0)
            {
                initial_repeat_delay = 0x0B;
                repeat_interval = 3;
            }
            else
            {
                initial_repeat_delay = 0x16;
                repeat_interval = 6;
            }
            one = 1;
            if (held_buttons & 0x10)
            {
                if ((port->pressed_buttons & 0x10) && (controller_base[0x1AA] == 0))
                {
                    port->face_repeat_timer_0 = initial_repeat_delay;
                }
                else
                {
                    delta = port->face_repeat_timer_0 - one;
                    if (delta <= 0)
                    {
                        delta = repeat_interval;
                        port->repeat_buttons |= 0x10;
                    }
                    port->face_repeat_timer_0 = delta;
                }
            }
            if (held_buttons & 0x20)
            {
                if ((port->pressed_buttons & 0x20) && (controller_base[0x1AA] == 0))
                {
                    port->face_repeat_timer_1 = initial_repeat_delay;
                }
                else
                {
                    delta = port->face_repeat_timer_1 - one;
                    if (delta <= 0)
                    {
                        delta = repeat_interval;
                        port->repeat_buttons |= 0x20;
                    }
                    port->face_repeat_timer_1 = delta;
                }
            }
            if (held_buttons & 0x40)
            {
                if ((port->pressed_buttons & 0x40) && (controller_base[0x1AA] == 0))
                {
                    port->face_repeat_timer_2 = initial_repeat_delay;
                }
                else
                {
                    delta = port->face_repeat_timer_2 - one;
                    if (delta <= 0)
                    {
                        delta = repeat_interval;
                        port->repeat_buttons |= 0x40;
                    }
                    port->face_repeat_timer_2 = delta;
                }
            }
            if (held_buttons & 0x80)
            {
                if ((port->pressed_buttons & 0x80) && (controller_base[0x1AA] == 0))
                {
                    port->face_repeat_timer_3 = initial_repeat_delay;
                }
                else
                {
                    delta = port->face_repeat_timer_3 - one;
                    if (delta <= 0)
                    {
                        delta = repeat_interval;
                        port->repeat_buttons |= 0x80;
                    }
                    port->face_repeat_timer_3 = delta;
                }
            }
            if (device_type != 0)
            {
                delta = raw_pad_data[4] - port->right_stick_center_x;
                if (((u32)(delta + 0x38)) < 0x71U)
                {
                    delta = 0;
                }
                if (delta < (-0x80))
                {
                    delta = -0x80;
                }
                else if (delta >= 0x80)
                {
                    delta = 0x7F;
                }
                shifted_delta = delta >> 4;
                if (delta < 0)
                {
                    port->right_stick_x = (delta + 0xF) >> 4;
                }
                else
                {
                    port->right_stick_x = shifted_delta;
                }
                delta = raw_pad_data[5] - port->right_stick_center_y;
                if (((u32)(delta + 0x38)) < 0x71U)
                {
                    delta = 0;
                }
                if (delta < (-0x80))
                {
                    delta = -0x80;
                }
                else if (delta >= 0x80)
                {
                    delta = 0x7F;
                }
                unsigned_temp = delta >> 4;
                if (delta < 0)
                {
                    port->right_stick_y = (delta + 0xF) >> 4;
                }
                else
                {
                    port->right_stick_y = unsigned_temp;
                }
                delta = raw_pad_data[6] - port->left_stick_center_x;
                if (((u32)(delta + 0x38)) < 0x71U)
                {
                    delta = 0;
                }
                if (delta < (-0x80))
                {
                    delta = -0x80;
                }
                else if (delta >= 0x80)
                {
                    delta = 0x7F;
                }
                shifted_delta = delta >> 4;
                if (delta < 0)
                {
                    shifted_delta = (delta + 0xF) >> 4;
                }
                delta = shifted_delta;
                port->left_stick_x = delta;
                direction_mask = 0x80;
                if (delta >= 0)
                {
                    if (delta > 0)
                    {
                        direction_mask = 0x20;
                    }
                    else
                    {
                        direction_mask = 0;
                    }
                }
                delta = raw_pad_data[7] - port->left_stick_center_y;
                if (((u32)(delta + 0x38)) < 0x71U)
                {
                    delta = 0;
                }
                if (delta < (-0x80))
                {
                    delta = -0x80;
                }
                else if (delta >= 0x80)
                {
                    delta = 0x7F;
                }
                shifted_delta = delta >> 4;
                if (delta < 0)
                {
                    shifted_delta = (delta + 0xF) >> 4;
                }
                delta = shifted_delta;
                port->left_stick_y = delta;
                if (delta < 0)
                {
                    direction_mask |= 0x10;
                }
                else if (delta > 0)
                {
                    direction_mask |= 0x40;
                }
                delta = direction_mask & (direction_mask ^ ((port->analog_direction_bits & 0xF) * 0x10));
                new_direction_bits = delta;
                direction_bits = new_direction_bits | ((direction_mask & 0xFF) >> 4);
                if (direction_mask & 0x10)
                {
                    if ((new_direction_bits & 0x10) && (controller_base[0x1AA] == 0))
                    {
                        port->direction_repeat_timer_0 = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->direction_repeat_timer_0 - one;
                        if (delta <= 0)
                        {
                            direction_bits |= 0x10;
                            delta = repeat_interval;
                        }
                        port->direction_repeat_timer_0 = delta;
                    }
                }
                if (direction_mask & 0x20)
                {
                    if ((new_direction_bits & 0x20) && (controller_base[0x1AA] == 0))
                    {
                        port->direction_repeat_timer_1 = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->direction_repeat_timer_1 - one;
                        if (delta <= 0)
                        {
                            direction_bits |= 0x20;
                            delta = repeat_interval;
                        }
                        port->direction_repeat_timer_1 = delta;
                    }
                }
                if (direction_mask & 0x40)
                {
                    if ((new_direction_bits & 0x40) && (controller_base[0x1AA] == 0))
                    {
                        port->direction_repeat_timer_2 = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->direction_repeat_timer_2 - one;
                        if (delta <= 0)
                        {
                            direction_bits |= 0x40;
                            delta = repeat_interval;
                        }
                        port->direction_repeat_timer_2 = delta;
                    }
                }
                if (direction_mask & 0x80)
                {
                    if ((new_direction_bits & 0x80) && (controller_base[0x1AA] == 0))
                    {
                        port->direction_repeat_timer_3 = initial_repeat_delay;
                    }
                    else
                    {
                        delta = port->direction_repeat_timer_3 - one;
                        if (delta <= 0)
                        {
                            direction_bits |= 0x80;
                            delta = repeat_interval;
                        }
                        port->direction_repeat_timer_3 = delta;
                    }
                }
                port->analog_direction_bits = direction_bits;
                return;
            }
            }
            else
            {
                port->device_type = 0xFF;
                return;
            }
        }
        else
        {
            port->device_type = 0xFF;
            return;
        }
    }
    else
    {
        clear_controller_sample(((u8*)port) + 0x20);
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
    u8* controller_base = (u8*)0x801ED600;

    if (PadChkVsync() != 0)
    {
        actuator_power_total = 0;
        controller_base[0x1A9] = (controller_base[0x1A9] + 1) & 0xF;
        poll_controller_port((ControllerPortState*)controller_base, &actuator_power_total);
        poll_controller_port((ControllerPortState*)(controller_base + 0xAE), &actuator_power_total);
        controller_base[0x1AA] = 0;
        return;
    }
    clear_controller_sample((void*)(controller_base + 0x20));
    clear_controller_sample((void*)(controller_base + 0xCE));
    controller_base[0x1AA] = 1;
}

/**
 * @brief Clear the mutable fields of a 16-byte controller sample.
 * @param sample Controller sample to clear.
 * @see decomp.me (100%) https://decomp.me/scratch/TSmff
 */
void clear_controller_sample(void* sample)
{
    unsigned char* bytes = (unsigned char*)sample;
    unsigned short* halfwords = (unsigned short*)sample;

    halfwords[3] = 0; // sh zero,6(a0)
    halfwords[2] = 0; // sh zero,4(a0)
    halfwords[1] = 0; // sh zero,2(a0)
    bytes[1] = 0;  // sb zero,1(a0)
    halfwords[7] = 0; // sh zero,0xe(a0)
    halfwords[6] = 0; // sh zero,0xc(a0)
    halfwords[5] = 0; // sh zero,0xa(a0)
    halfwords[4] = 0; // sh zero,8(a0)
}

/**
 * @brief Stop LIBPAD communication and restore the previous VSync callback.
 * @see decomp.me (100%) https://decomp.me/scratch/NkZqx
 */
void shutdown_controllers(void)
{
    u8* controller_base = (u8*)0x801ED600;
    VSync(0);
    PadStopCom();
    VSyncCallback(*(void (**)(void))(controller_base + 0x1A4));
    controller_base[0x1AA] = 0;
}

/**
 * @brief Install the controller VSync callback and reset its sample counters.
 * @see decomp.me (100%) https://decomp.me/scratch/BwqlO
 */
void initialize_controller_vsync(void)
{
    u8* controller_base = (u8*)0x801ED600;
    VSync(0);
    VSyncCallback(controller_vsync_callback);
    controller_base[0x1A2] = 0;
    controller_base[0x1A1] = 0;
}

/**
 * @brief Set the number of VSyncs over which controller samples are accumulated.
 * @param interval VSync accumulation interval; zero leaves rollover disabled.
 * @see decomp.me (100%) https://decomp.me/scratch/zw8m7
 */
void set_controller_vsync_interval(unsigned long interval)
{
    u8* controller_base = (u8*)0x801ED600;
    if (controller_base[0x1A2] >= interval)
    {
        controller_base[0x1A2] = 0;
        return;
    }
    controller_base[0x1A3] = interval;
}

/**
 * @brief Publish the latest controller state and transfer queued VSync samples to history.
 * @see decomp.me (100%) https://decomp.me/scratch/GawXK
 */
void update_controllers(void)
{
    u8* source_sample;
    u8* controller_base;
    s32 sample_index;
    s32 port1_dest_offset;
    s32 port2_dest_offset;
    s32 port1_source_offset;
    s32 port2_source_offset;
    u8 sample_count;
    s32 history_offset;
    s32 loop_end;
    controller_poll();
    controller_base = (u8*)0x801ED600;
    if (g_controller_vsync_sample_count == 0)
    {
        clear_controller_sample((void*)(controller_base + 0x10));
        clear_controller_sample((void*)(controller_base + 0xBE));
    }
    merge_latest_controller_sample((void*)controller_base);
    merge_latest_controller_sample((void*)(controller_base + 0xAE));
    sample_index = controller_base[0x1A1];
    sample_count = sample_index;
    controller_base[0x1A0] = sample_count;
    sample_index = sample_count - 1;
    if (sample_index != (-1))
    {
        loop_end = -1;
        history_offset = sample_index * 0x10;
        port2_source_offset = history_offset + 0x10E;
        port1_source_offset = history_offset + 0x60;
        port2_dest_offset = 0xDE;
        port1_dest_offset = 0x30;
        do
        {
            source_sample = controller_base + port2_source_offset;
            copy_controller_sample((void*)(controller_base + port1_source_offset), (void*)(controller_base + port1_dest_offset));
            copy_controller_sample((void*)source_sample, (void*)(controller_base + port2_dest_offset));
            port2_dest_offset += 0x10;
            port1_dest_offset += 0x10;
            port2_source_offset -= 0x10;
            sample_index -= 1;
            port1_source_offset -= 0x10;
        } while (sample_index != loop_end);
    }
    controller_base[0x1A1] = 0;
    controller_base[0x1A2] = 1;
    controller_base[0x1A3] = 0;
}

/**
 * @brief Reset controller VSync accumulation without stopping LIBPAD communication.
 * @see decomp.me (100%) https://decomp.me/scratch/RJyNE
 */
void reset_controller_vsync_state(void)
{
    u8* controller_base = (u8*)0x801ED600;

    controller_base[0x1A2] = 0;
    controller_base[0x1A0] = 0;
    controller_base[0x1A1] = 0;
    controller_base[0x1AA] = 0;
}

/**
 * @brief Poll and accumulate controller samples from the installed VSync callback.
 * @see decomp.me (100%) https://decomp.me/scratch/srP3p
 */
void controller_vsync_callback(void)
{
    u8* controller_base = (u8*)0x801ED600;

    if (g_controller_vsync_counter != 0)
    {
        controller_poll();

        if (controller_base[0x1A1] < 3)
        {
            copy_controller_sample((void*)(controller_base + 0x20), (void*)(controller_base + ((controller_base[0x1A1] << 4) + 0x60)));
            copy_controller_sample((void*)(controller_base + 0xCE), (void*)(controller_base + ((controller_base[0x1A1] << 4) + 0x10E)));
            controller_base[0x1A1] = controller_base[0x1A1] + 1;
        }

        if (controller_base[0x1A2] == 1)
        {
            clear_controller_sample((void*)(controller_base + 0x10));
            clear_controller_sample((void*)(controller_base + 0xBE));
        }

        accumulate_controller_sample((void*)controller_base);
        accumulate_controller_sample((void*)(controller_base + 0xAE));

        {
            u8 next_counter = controller_base[0x1A2] + 1;
            u8 interval = controller_base[0x1A3];
            controller_base[0x1A2] = next_counter;
            if (interval != 0 && next_counter >= interval)
            {
                controller_base[0x1A2] = 0;
            }
        }
    }
}

/**
 * @brief Accumulate the latest sample into a port's interval aggregate.
 * @param port_state Base of the per-port controller state.
 * @see decomp.me (100%) https://decomp.me/scratch/aj1vL
 */
void accumulate_controller_sample(void* port_state)
{
    u8* port_bytes = (u8*)port_state;
    s32 device_type = (s32)port_bytes[0x20];
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
        *((u16*)(port_bytes + 0x18)) += *((u16*)(port_bytes + 0x28));
        *((u16*)(port_bytes + 0x1A)) += *((u16*)(((u8*)port_state) + 0x2A));
        *((u16*)(((u8*)port_state) + 0x1C)) += *((u16*)(((u8*)port_state) + 0x2C));
        *((u16*)(((u8*)port_state) + 0x1E)) += *((u16*)(((u8*)port_state) + 0x2E));
        ((u8*)port_state)[0x11] |= ((u8*)port_state)[0x21];
    }
    *((u16*)(((u8*)port_state) + 0x12)) |= *((u16*)(((u8*)port_state) + 0x22));
    *((u16*)(((u8*)port_state) + 0x14)) |= *((u16*)(((u8*)port_state) + 0x24));
    *((u16*)(((u8*)port_state) + 0x16)) |= *((u16*)(((u8*)port_state) + 0x26));
}

/**
 * @brief Merge the accumulated and latest samples into a port's published sample.
 * @param port_state Base of the per-port controller state.
 * @see decomp.me (100%) https://decomp.me/scratch/ORRFA
 */
void merge_latest_controller_sample(void* port_state)
{
    s32 device_type = ((u8*)port_state)[0x20];
    ((u8*)port_state)[0] = device_type;
    device_type &= 0xFF;

    if (device_type == 0)
    {
        /* Empty branch forces a nop in the delay slot. */
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
        *(u16*)((u8*)port_state + 8) = *(u16*)((u8*)port_state + 0x18) + *(u16*)((u8*)port_state + 0x28);
        *(u16*)((u8*)port_state + 0xA) = *(u16*)((u8*)port_state + 0x1A) + *(u16*)((u8*)port_state + 0x2A);
        *(u16*)((u8*)port_state + 0xC) = *(u16*)((u8*)port_state + 0x1C) + *(u16*)((u8*)port_state + 0x2C);
        *(u16*)((u8*)port_state + 0xE) = *(u16*)((u8*)port_state + 0x1E) + *(u16*)((u8*)port_state + 0x2E);
        ((u8*)port_state)[1] = ((u8*)port_state)[0x11] | ((u8*)port_state)[0x21];
    }

    *(u16*)((u8*)port_state + 2) = *(u16*)((u8*)port_state + 0x12) | *(u16*)((u8*)port_state + 0x22);
    *(u16*)((u8*)port_state + 4) = *(u16*)((u8*)port_state + 0x14) | *(u16*)((u8*)port_state + 0x24);
    *(u16*)((u8*)port_state + 6) = *(u16*)((u8*)port_state + 0x16) | *(u16*)((u8*)port_state + 0x26);
}

/**
 * @brief Copy one 16-byte controller sample while preserving device-type handling.
 * @param source Controller sample to copy.
 * @param destination Controller sample to overwrite.
 * @see decomp.me (100%) https://decomp.me/scratch/Hkz5t
 */
void copy_controller_sample(void* source, void* destination)
{
    u8* src = (u8*)source;
    u8* dst = (u8*)destination;
    s32 device_type;

    clear_controller_sample(dst);
    dst[0] = src[0];
    device_type = (s32)src[0]; /* second load for comparison */

    if (device_type == 0)
    {
        /* Empty branch forces a nop in the beqz delay slot. */
    }
    else
    {
        if (device_type < 0)
        {
            return; /* bltz */
        }
        if (device_type >= 3)
        {
            return; /* slti + beqz with nop */
        }
        *(u16*)(dst + 8) = *(u16*)(src + 8);
        *(u16*)(dst + 10) = *(u16*)(src + 10);
        *(u16*)(dst + 12) = *(u16*)(src + 12);
        *(u16*)(dst + 14) = *(u16*)(src + 14);
        dst[1] = src[1];
    }

    /* Button block - executed for device_type 0, 1, or 2. */
    *(u16*)(dst + 2) = *(u16*)(src + 2);
    *(u16*)(dst + 4) = *(u16*)(src + 4);
    *(u16*)(dst + 6) = *(u16*)(src + 6);
}
