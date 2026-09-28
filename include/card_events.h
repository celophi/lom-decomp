#ifndef CARD_EVENTS_H
#define CARD_EVENTS_H

#include "common.h"

/**
 * @file card_events.h
 * @brief Memory-card event handling compiled into ADDHERO, CARDA, CLOAD and NIKI.
 *
 * Psy-Q reports memory-card results through two groups of events, software
 * (SwCARD) and hardware (HwCARD), each with I/O complete, error, timeout and
 * new-card events. Each overlay includes these functions from
 * src/common/card_events/<function>.inc.c at the point where they sit in its binary
 * and maps the globals below to its own data, so every overlay still links its
 * own copy.
 */

/** @brief Memory-card channel of card slot @p slot (port in the high nibble). */
#define CARD_CHANNEL(slot) ((slot) * 0x10)

/** @brief Result of polling a card event group, in the order the events are tested. */
typedef enum
{
    CARD_EVENT_NONE = -1,    /**< No event is pending. */
    CARD_EVENT_COMPLETE = 0, /**< The operation completed. */
    CARD_EVENT_ERROR = 1,    /**< The card reported an error. */
    CARD_EVENT_TIMEOUT = 2,  /**< No card, or no answer. */
    CARD_EVENT_NEW_CARD = 3  /**< A different card was inserted. */
} CardEvent;

/** @brief Card slot (0 or 1) the overlay is working with. */
extern s32 g_card_slot;

/** @brief Current command in the overlay's card sequence bytecode. */
extern u8* g_card_step;

/** @brief Step table the sequence waits on after a restart. */
extern u8 g_card_steps_idle[];

/** @brief Software (SwCARD) event handles. */
extern s32 g_card_software_event_io_complete;
extern s32 g_card_software_event_error;
extern s32 g_card_software_event_timeout;
extern s32 g_card_software_event_new_card;

/** @brief Hardware (HwCARD) event handles. */
extern s32 g_card_hardware_event_io_complete;
extern s32 g_card_hardware_event_error;
extern s32 g_card_hardware_event_timeout;
extern s32 g_card_hardware_event_new_card;

void restart_card_sequence(void);
s32 poll_and_retry_card_info(void);
void shutdown_card_events(void);
void clear_software_card_events(void);
void clear_hardware_card_events(void);
s32 poll_software_card_events(void);
s32 poll_hardware_card_events(void);

#endif
