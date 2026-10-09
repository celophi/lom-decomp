#include "common/card_menu.h"

/**
 * @brief Set up card events and clear the transfer flags.
 */
void card_menu_init_card_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_card_software_event_io_complete = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_card_software_event_error = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_card_software_event_timeout = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_card_software_event_new_card = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, 0);
    g_card_hardware_event_io_complete = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, 0);
    g_card_hardware_event_error = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, 0);
    g_card_hardware_event_timeout = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    g_card_hardware_event_new_card = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, 0);
    EnableEvent(g_card_software_event_io_complete);
    EnableEvent(g_card_software_event_error);
    EnableEvent(g_card_software_event_timeout);
    EnableEvent(g_card_software_event_new_card);
    EnableEvent(g_card_hardware_event_io_complete);
    EnableEvent(g_card_hardware_event_error);
    EnableEvent(g_card_hardware_event_timeout);
    EnableEvent(g_card_hardware_event_new_card);
    ExitCriticalSection();
    g_card_menu_progress_bar_active = 0;
    g_card_menu_entry_scan_active = 0;
}
