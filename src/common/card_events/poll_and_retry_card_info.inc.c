/*
 * Shared memory-card event function; see include/common/card_events.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/card_events.h"

/**
 * @brief Poll the software card events and request card information again when one has arrived.
 * @return The CardEvent that arrived, or CARD_EVENT_NONE.
 */
s32 poll_and_retry_card_info(void)
{
    s32 event_status;

    event_status = poll_software_card_events();
    if (event_status != CARD_EVENT_NONE)
    {
        _card_wait(g_card_slot);
        _card_info(CARD_CHANNEL(g_card_slot));
    }
    return event_status;
}
