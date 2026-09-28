/*
 * Shared memory-card event function; see include/card_events.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "card_events.h"

/**
 * @brief Close the software and hardware memory-card events.
 */
void shutdown_card_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(g_card_software_event_io_complete);
    CloseEvent(g_card_software_event_error);
    CloseEvent(g_card_software_event_timeout);
    CloseEvent(g_card_software_event_new_card);
    CloseEvent(g_card_hardware_event_io_complete);
    CloseEvent(g_card_hardware_event_error);
    CloseEvent(g_card_hardware_event_timeout);
    CloseEvent(g_card_hardware_event_new_card);
    ExitCriticalSection();
}
