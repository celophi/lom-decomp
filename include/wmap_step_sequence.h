#ifndef WMAP_STEP_SEQUENCE_H
#define WMAP_STEP_SEQUENCE_H

/*
 * Definitions of the step functions every WMAP effect sequence repeats. Each
 * macro defines one function; the model and naming are described with
 * WmapSequenceCallback in wmap_sequence_runtime.h. Steps that do more than
 * these patterns are written out in their own files.
 */

#include "wmap_sequence_runtime.h"

/**
 * @brief Define a step-sequence runner: reset on request, else run the current step.
 * @param name Runner function name.
 * @param table Step table the runner indexes.
 * @param count Number of entries in @p table.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 */
#define WMAP_STEP_RUNNER(name, table, count, step, timer) \
    s32 name(s32 arg0)                                    \
    {                                                     \
        s32 result;                                       \
                                                          \
        if (arg0 != 0)                                    \
        {                                                 \
            step = 1;                                     \
            timer = 1;                                    \
            return 1;                                     \
        }                                                 \
                                                          \
        if (step < count)                                 \
        {                                                 \
            table[step]();                                \
            result = 1;                                   \
        }                                                 \
        else                                              \
        {                                                 \
            result = 0;                                   \
        }                                                 \
        return result;                                    \
    }

/**
 * @brief Define a runner whose reset also runs step 1 in the same call.
 * @param name Runner function name.
 * @param table Step table the runner indexes.
 * @param count Number of entries in @p table.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @note Unlike WMAP_STEP_RUNNER, a reset request does not return early.
 */
#define WMAP_STEP_RUNNER_RESET_AND_RUN(name, table, count, step, timer) \
    s32 name(s32 arg0)                                                  \
    {                                                                   \
        s32 result;                                                     \
                                                                        \
        if (arg0 != 0)                                                  \
        {                                                               \
            step = 1;                                                   \
            timer = 1;                                                  \
        }                                                               \
                                                                        \
        if (step < count)                                               \
        {                                                               \
            table[step]();                                              \
            result = 1;                                                 \
        }                                                               \
        else                                                            \
        {                                                               \
            result = 0;                                                 \
        }                                                               \
        return result;                                                  \
    }

/**
 * @brief Define a step 0 that restarts its sequence at step 1 with a one-frame timer.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 */
#define WMAP_STEP_RESET(name, step, timer) \
    void name(void)                        \
    {                                      \
        step = 1;                          \
        timer = 1;                         \
    }

/**
 * @brief Define a step that counts the timer down, then advances.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 */
#define WMAP_STEP_WAIT(name, step, timer) \
    void name(void)                       \
    {                                     \
        if (--timer == 0)                 \
        {                                 \
            step += 1;                    \
        }                                 \
    }

/**
 * @brief Define a step that advances unconditionally; as the last entry it ends the sequence.
 * @param name Step function name.
 * @param step The sequence's step global.
 */
#define WMAP_STEP_ADVANCE(name, step) \
    void name(void)                   \
    {                                 \
        step += 1;                    \
    }

/**
 * @brief Define a step that advances and runs the next step once g_wmap_sequence_busy clears.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param next The next step, run in the same frame.
 */
#define WMAP_STEP_WAIT_IDLE(name, step, next) \
    void name(void)                           \
    {                                         \
        if (g_wmap_sequence_busy == 0)        \
        {                                     \
            step += 1;                        \
            next();                           \
        }                                     \
    }

#endif
