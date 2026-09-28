/*
 * Shared memory-card event function; see include/card_events.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "card_events.h"

/**
 * @brief Consume the first pending software memory-card event.
 * @return The CardEvent consumed, or CARD_EVENT_NONE when none is pending.
 */
s32 poll_software_card_events(void)
{
    if (TestEvent(g_card_software_event_io_complete) == 1)
    {
        return CARD_EVENT_COMPLETE;
    }
    if (TestEvent(g_card_software_event_error) == 1)
    {
        return CARD_EVENT_ERROR;
    }
    if (TestEvent(g_card_software_event_timeout) == 1)
    {
        return CARD_EVENT_TIMEOUT;
    }
    if (TestEvent(g_card_software_event_new_card) == 1)
    {
        return CARD_EVENT_NEW_CARD;
    }
    return CARD_EVENT_NONE;
}
