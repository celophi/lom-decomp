/*
 * Shared memory-card event function; see include/common/card_events.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/card_events.h"

/**
 * @brief Clear the software events, request fresh card information and park the
 *        card sequence on its idle step table.
 */
void restart_card_sequence(void)
{
    _card_wait(g_card_slot);
    clear_software_card_events();
    _card_info(CARD_CHANNEL(g_card_slot));
    g_card_step = g_card_steps_idle;
}
