/*
 * Shared memory-card event function; see include/common/card_events.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/card_events.h"

/**
 * @brief Consume any pending software memory-card events.
 */
void clear_software_card_events(void)
{
    TestEvent(g_card_software_event_io_complete);
    TestEvent(g_card_software_event_error);
    TestEvent(g_card_software_event_timeout);
    TestEvent(g_card_software_event_new_card);
}
