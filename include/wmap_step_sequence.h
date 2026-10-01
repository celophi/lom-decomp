#ifndef WMAP_STEP_SEQUENCE_H
#define WMAP_STEP_SEQUENCE_H

/*
 * Shared source for repeated WMAP sequence steps and model fade calculations.
 * The runner model and naming are described with WmapSequenceCallback in
 * wmap_sequence_runtime.h. Steps with different drawing or update behavior
 * keep that code in their own files.
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
#define WMAP_STEP_RUNNER(name, table, count, step, timer)                                                                                                      \
    s32 name(s32 arg0)                                                                                                                                         \
    {                                                                                                                                                          \
        s32 result;                                                                                                                                            \
                                                                                                                                                               \
        if (arg0 != 0)                                                                                                                                         \
        {                                                                                                                                                      \
            step = 1;                                                                                                                                          \
            timer = 1;                                                                                                                                         \
            return 1;                                                                                                                                          \
        }                                                                                                                                                      \
                                                                                                                                                               \
        if (step < count)                                                                                                                                      \
        {                                                                                                                                                      \
            table[step]();                                                                                                                                     \
            result = 1;                                                                                                                                        \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            result = 0;                                                                                                                                        \
        }                                                                                                                                                      \
        return result;                                                                                                                                         \
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
#define WMAP_STEP_RUNNER_RESET_AND_RUN(name, table, count, step, timer)                                                                                        \
    s32 name(s32 arg0)                                                                                                                                         \
    {                                                                                                                                                          \
        s32 result;                                                                                                                                            \
                                                                                                                                                               \
        if (arg0 != 0)                                                                                                                                         \
        {                                                                                                                                                      \
            step = 1;                                                                                                                                          \
            timer = 1;                                                                                                                                         \
        }                                                                                                                                                      \
                                                                                                                                                               \
        if (step < count)                                                                                                                                      \
        {                                                                                                                                                      \
            table[step]();                                                                                                                                     \
            result = 1;                                                                                                                                        \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            result = 0;                                                                                                                                        \
        }                                                                                                                                                      \
        return result;                                                                                                                                         \
    }

/**
 * @brief Define a step 0 that restarts its sequence at step 1 with a one-frame timer.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 */
#define WMAP_STEP_RESET(name, step, timer)                                                                                                                     \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        step = 1;                                                                                                                                              \
        timer = 1;                                                                                                                                             \
    }

/**
 * @brief Define a step that counts the timer down, then advances.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 */
#define WMAP_STEP_WAIT(name, step, timer)                                                                                                                      \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        if (--timer == 0)                                                                                                                                      \
        {                                                                                                                                                      \
            step += 1;                                                                                                                                         \
        }                                                                                                                                                      \
    }

/**
 * @brief Define a step that updates an effect each frame until its timer expires.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param update_call Function call to run once before counting down.
 */
#define WMAP_STEP_UPDATE_AND_WAIT(name, step, timer, update_call)                        \
    void name(void)                                                                      \
    {                                                                                    \
        update_call;                                                                     \
        if (--(timer) == 0)                                                              \
        {                                                                                \
            (step) += 1;                                                                 \
        }                                                                                \
    }

/**
 * @brief Define a step that updates an effect and increases a value each frame.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param value Shared value to increase after the update.
 * @param amount Amount added each frame.
 * @param update_call Function call to run before increasing the value.
 */
#define WMAP_STEP_UPDATE_AND_RAMP(name, step, timer, value, amount, update_call)         \
    void name(void)                                                                      \
    {                                                                                    \
        s32 remaining;                                                                   \
                                                                                         \
        update_call;                                                                     \
        (value) += (amount);                                                             \
        remaining = (timer) - 1;                                                         \
        (timer) = remaining;                                                             \
        if (remaining == 0)                                                              \
        {                                                                                \
            (step) += 1;                                                                 \
        }                                                                                \
    }

/**
 * @brief Define a step that runs two updates in order, then counts down.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param first_call Function call to run first each frame.
 * @param second_call Function call to run second each frame.
 */
#define WMAP_STEP_UPDATE_TWO_AND_WAIT(name, step, timer, first_call, second_call)        \
    void name(void)                                                                      \
    {                                                                                    \
        first_call;                                                                      \
        second_call;                                                                     \
        if (--(timer) == 0)                                                              \
        {                                                                                \
            (step) += 1;                                                                 \
        }                                                                                \
    }

/** Sprite scale-table entry used when a sequence starts an actor. */
#define WMAP_STEP_ACTOR_SCALE_INDEX 15

/**
 * @brief Define a step that starts a sprite animation and runs its first update.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param slot Shared index of the sprite actor and its animation state.
 * @param animation Animation data assigned to the slot.
 * @param sequence_id Constant animation sequence to start.
 * @param initial_shade Starting shade.
 * @param shade_target Shade the animation update moves toward.
 * @param shade_delta Constant amount the animation update changes the shade each frame.
 * @param frames Frames the following update step runs.
 * @param next Update step, called immediately after advancing.
 * @note The constant conditions preserve store order without runtime branches.
 */
#define WMAP_STEP_START_ACTOR(name, step, timer, slot, animation, sequence_id, initial_shade, shade_target, shade_delta, frames, next) \
    void name(void)                                                                      \
    {                                                                                    \
        WmapSpriteActor* actor = &g_wmap_sprite_actors[(slot)];                          \
                                                                                         \
        g_wmap_actor_animations[(slot)].data = (animation);                              \
        actor->scale_index = WMAP_STEP_ACTOR_SCALE_INDEX;                                \
        if ((sequence_id) != 0)                                                          \
        {                                                                                \
            actor->sequence = (sequence_id);                                             \
        }                                                                                \
        actor->previous_sequence = -1;                                                   \
        if ((shade_delta) != 0)                                                          \
        {                                                                                \
            actor->shade_step = (shade_delta);                                           \
        }                                                                                \
        actor->resource_index = 0;                                                       \
        if ((sequence_id) == 0)                                                          \
        {                                                                                \
            actor->sequence = 0;                                                         \
        }                                                                                \
        if ((shade_delta) == 0)                                                          \
        {                                                                                \
            actor->shade_step = 0;                                                       \
        }                                                                                \
        actor->target_shade = (shade_target);                                            \
        actor->shade = (initial_shade);                                                  \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
        next();                                                                          \
    }

/**
 * @brief Define a step that starts a sprite fade and runs its first update.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param actor_state Sprite actor to fade to zero.
 * @param shade_delta Amount the animation update subtracts from its shade.
 * @param frames Frames the following update step runs.
 * @param next Update step, called immediately after advancing.
 */
#define WMAP_STEP_FADE_ACTOR(name, step, timer, actor_state, shade_delta, frames, next)  \
    void name(void)                                                                      \
    {                                                                                    \
        WmapSpriteActor* actor = &(actor_state);                                         \
                                                                                         \
        actor->shade_step = (shade_delta);                                               \
        actor->target_shade = 0;                                                         \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
        next();                                                                          \
    }

/**
 * @brief Define a step that fades a range of sprite actors to zero.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param first First actor slot to fade.
 * @param end First actor slot after the range.
 * @param shade_delta Amount the animation update subtracts from each actor's shade.
 * @param frames Frames the following update step runs.
 * @param next Update step, called immediately after advancing.
 */
#define WMAP_STEP_FADE_ACTOR_RANGE(name, step, timer, first, end, shade_delta, frames, next) \
    void name(void)                                                                      \
    {                                                                                    \
        s32 i;                                                                           \
                                                                                         \
        for (i = (first); i < (end); i++)                                                \
        {                                                                                \
            g_wmap_sprite_actors[i].target_shade = 0;                                    \
            g_wmap_sprite_actors[i].shade_step = (shade_delta);                          \
        }                                                                                \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
        next();                                                                          \
    }

/**
 * @brief Define a step that stops spawning particles and keeps updating them.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param spawn_interval Emitter interval field; -1 disables new particles.
 * @param frames Frames to keep updating the existing particles.
 * @param next Update step, called immediately after advancing.
 */
#define WMAP_STEP_STOP_EMITTER(name, step, timer, spawn_interval, frames, next)          \
    void name(void)                                                                      \
    {                                                                                    \
        (timer) = (frames);                                                              \
        (spawn_interval) = -1;                                                           \
        (step) += 1;                                                                     \
        next();                                                                          \
    }

/**
 * @brief Define a step that clears a particle group's remaining spawn count.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param spawn_count Remaining spawn count; zero disables new particles.
 * @param frames Frames to keep updating the existing particles.
 * @param next Update step, called immediately after advancing.
 */
#define WMAP_STEP_STOP_PARTICLE_SPAWNS(name, step, timer, spawn_count, frames, next)     \
    void name(void)                                                                      \
    {                                                                                    \
        (spawn_count) = 0;                                                               \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
        next();                                                                          \
    }

/**
 * @brief Define a step that advances unconditionally; as the last entry it ends the sequence.
 * @param name Step function name.
 * @param step The sequence's step global.
 */
#define WMAP_STEP_ADVANCE(name, step)                                                                                                                          \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        step += 1;                                                                                                                                             \
    }

/**
 * @brief Define a step that advances and runs the next step once g_wmap_sequence_busy clears.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param next The next step, run in the same frame.
 */
#define WMAP_STEP_WAIT_IDLE(name, step, next)                                                                                                                  \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        if (g_wmap_sequence_busy == 0)                                                                                                                         \
        {                                                                                                                                                      \
            step += 1;                                                                                                                                         \
            next();                                                                                                                                            \
        }                                                                                                                                                      \
    }

/** @brief Scroll mode used while a sequence moves the map to a target. */
#define WMAP_STEP_SCRIPTED_SCROLL_MODE 2

/**
 * @brief Define a step that waits for scripted map scrolling, then runs the next step.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param next Step to run once the scroll mode clears.
 */
#define WMAP_STEP_WAIT_SCROLL(name, step, next)                                          \
    void name(void)                                                                      \
    {                                                                                    \
        if (g_wmap_view_scroll_mode != WMAP_STEP_SCRIPTED_SCROLL_MODE)                   \
        {                                                                                \
            (step) += 1;                                                                 \
            next();                                                                      \
        }                                                                                \
    }

/**
 * @brief Define a step that starts another sequence, then waits @p frames frames.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param runner Runner of the sequence to start.
 * @param frames Frames the next (wait) step counts down.
 */
#define WMAP_STEP_START_AND_WAIT(name, step, timer, runner, frames)                                                                                            \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        wmap_start_sequence(runner);                                                                                                                           \
        timer = frames;                                                                                                                                        \
        step += 1;                                                                                                                                             \
    }

/**
 * @brief Define a step that starts two sequences, then waits @p frames frames.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param first Runner of the first sequence to start.
 * @param second Runner of the second sequence to start.
 * @param frames Frames the next (wait) step counts down.
 */
#define WMAP_STEP_START_TWO_AND_WAIT(name, step, timer, first, second, frames)                                                                                 \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        wmap_start_sequence(first);                                                                                                                            \
        wmap_start_sequence(second);                                                                                                                           \
        timer = frames;                                                                                                                                        \
        step += 1;                                                                                                                                             \
    }

/**
 * @brief Define a step that starts three sequences in order and sets the wait timer.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param first Runner of the first sequence to start.
 * @param second Runner of the second sequence to start.
 * @param third Runner of the third sequence to start.
 * @param frames Frames the next step waits.
 */
#define WMAP_STEP_START_THREE_AND_WAIT(name, step, timer, first, second, third, frames)  \
    void name(void)                                                                      \
    {                                                                                    \
        wmap_start_sequence(first);                                                      \
        wmap_start_sequence(second);                                                     \
        wmap_start_sequence(third);                                                      \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
    }

/**
 * @brief Define a step that hides an overlay or transition mesh and sets the wait timer.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param hidden Visibility flag to set; 1 hides the overlay or mesh.
 * @param frames Frames the next step waits.
 */
#define WMAP_STEP_HIDE_AND_WAIT(name, step, timer, hidden, frames)                       \
    void name(void)                                                                      \
    {                                                                                    \
        (hidden) = 1;                                                                    \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
    }

/**
 * @brief Define a step that hides an overlay, starts a sequence, and sets the wait timer.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param hidden Visibility flag to set before starting the sequence.
 * @param runner Runner of the sequence to start.
 * @param frames Frames the next step waits.
 */
#define WMAP_STEP_HIDE_AND_START(name, step, timer, hidden, runner, frames)              \
    void name(void)                                                                      \
    {                                                                                    \
        (hidden) = 1;                                                                    \
        wmap_start_sequence(runner);                                                     \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
    }

/**
 * @brief Define a step that sets the timer and runs the next step in the same frame.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param frames Frames the next step counts down.
 * @param next The next step.
 */
#define WMAP_STEP_ARM_TIMER(name, step, timer, frames, next)                                                                                                   \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        timer = frames;                                                                                                                                        \
        step += 1;                                                                                                                                             \
        next();                                                                                                                                                \
    }

/**
 * @brief Define a step that starts a blocking sequence, sets g_wmap_sequence_busy and moves on.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param runner Runner of the blocking sequence; it clears the flag when it finishes.
 * @param next The next step (usually a wait_idle), run in the same frame.
 */
#define WMAP_STEP_START_BLOCKING(name, step, runner, next)                                                                                                     \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        wmap_start_sequence(runner);                                                                                                                           \
        g_wmap_sequence_busy = 1;                                                                                                                              \
        step += 1;                                                                                                                                             \
        next();                                                                                                                                                \
    }

/**
 * @brief Define a step that animates and draws a sprite actor, then counts the timer down.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param actor Sprite actor to animate and draw.
 * @param animation Animation resource the actor steps through.
 * @param position Screen position (its packed form is drawn).
 * @param texture_index Texture-page and palette resource index.
 * @param ot_index Ordering-table index.
 * @param variant Scale-table selector.
 */
#define WMAP_STEP_DRAW_ACTOR_AND_WAIT(name, step, timer, actor, animation, position, texture_index, ot_index, variant)                                         \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        wmap_step_actor_animation(&actor, &animation);                                                                                                         \
        wmap_draw_actor_sprite(&actor, position.packed, texture_index, ot_index, variant);                                                                     \
        if (--timer == 0)                                                                                                                                      \
        {                                                                                                                                                      \
            step += 1;                                                                                                                                         \
        }                                                                                                                                                      \
    }

/**
 * @brief Increase a model's shade and clamp it to the maximum.
 * @param shade Model's RGB multiplier, in units of 1/128.
 * @param amount Amount added before drawing the next frame.
 * @param maximum Highest shade allowed.
 * @param result s32 scratch variable receiving the shade before clamping.
 * @note A do/while wrapper changes how the old compiler schedules the fade.
 */
#define WMAP_MODEL_FADE_IN(shade, amount, maximum, result)                               \
    {                                                                                    \
        (result) = (shade) + (amount);                                                   \
        (shade) = (result);                                                              \
        if ((result) > (maximum))                                                        \
        {                                                                                \
            (shade) = (maximum);                                                         \
        }                                                                                \
    }

/**
 * @brief Decrease a model's shade and clamp it to zero.
 * @param shade Model's RGB multiplier, in units of 1/128.
 * @param amount Amount subtracted before drawing the next frame.
 * @param result s32 scratch variable receiving the shade before clamping.
 */
#define WMAP_MODEL_FADE_OUT(shade, amount, result)                                       \
    {                                                                                    \
        (result) = (shade) - (amount);                                                   \
        (shade) = (result);                                                              \
        if ((result) < 0)                                                                \
        {                                                                                \
            (shade) = 0;                                                                 \
        }                                                                                \
    }

/**
 * @brief Define a step that initializes an animated model and runs its first update.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param rotation_copy Rotation used by the model update.
 * @param rotation Initial rotation, including any required type cast.
 * @param shade The model's shade global.
 * @param initial_shade Starting shade.
 * @param frame_counter Animation counter, reset to zero.
 * @param frames Frames the following update step runs.
 * @param next Update step, called immediately after advancing.
 */
#define WMAP_STEP_START_MODEL(name, step, timer, rotation_copy, rotation, shade, initial_shade, frame_counter, frames, next) \
    void name(void)                                                                      \
    {                                                                                    \
        (shade) = (initial_shade);                                                       \
        (rotation_copy) = (rotation);                                                    \
        (frame_counter) = 0;                                                             \
        (timer) = (frames);                                                              \
        (step) += 1;                                                                     \
        next();                                                                          \
    }

/**
 * @brief Define the first step of a model drop: place the model above the camera and start the drop.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer The sequence's timer global.
 * @param rotation_copy The drop's copy of @p rotation.
 * @param rotation Rotation the model takes.
 * @param position The model's position; it starts at the camera translation.
 * @param shade The model's shade global.
 * @param shade_value Starting shade.
 * @param start_z Starting height (Z) of the drop.
 * @param frames Frames the drop update runs.
 * @param next The drop update step, run in the same frame.
 */
#define WMAP_STEP_DROP_START(name, step, timer, rotation_copy, rotation, position, shade, shade_value, start_z, frames, next)                                  \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        rotation_copy = rotation;                                                                                                                              \
        position = g_wmap_camera_translation;                                                                                                                  \
        shade = shade_value;                                                                                                                                   \
        position.vz = start_z;                                                                                                                                 \
        timer = frames;                                                                                                                                        \
        step += 1;                                                                                                                                             \
        next();                                                                                                                                                \
    }

#define WMAP_DROP_MIN_Z 10000
#define WMAP_DROP_OT_INDEX 4
#define WMAP_DROP_TPAGE 0x35
#define WMAP_DROP_CLUT 0x7800
#define WMAP_DROP_BLEND_MODE 1

/**
 * @brief Define a model drop update with an explicit draw call.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer Frames left before advancing the step.
 * @param rotation The model's rotation.
 * @param position The model's position, clamped to WMAP_DROP_MIN_Z.
 * @param shade The model's shade global, clamped to zero after drawing.
 * @param z_delta Signed Z movement per frame.
 * @param shade_step Amount subtracted from the shade each frame.
 * @param draw_call Model draw call, including its screen offsets and render settings.
 */
#define WMAP_STEP_DROP_UPDATE_WITH_DRAW(name, step, timer, rotation, position, shade, z_delta, shade_step, draw_call) \
    void name(void)                                                                      \
    {                                                                                    \
        MATRIX transform;                                                                \
        s32 depth;                                                                       \
                                                                                         \
        depth = (position).vz + (z_delta);                                               \
        (position).vz = depth;                                                           \
        if (depth < WMAP_DROP_MIN_Z)                                                     \
        {                                                                                \
            (position).vz = WMAP_DROP_MIN_Z;                                             \
        }                                                                                \
                                                                                         \
        PushMatrix();                                                                    \
        RotMatrix(&(rotation), &transform);                                              \
        TransMatrix(&transform, &g_wmap_zero_translation);                               \
        SetRotMatrix(&transform);                                                        \
        SetTransMatrix(&transform);                                                      \
                                                                                         \
        if ((shade) != 0)                                                                \
        {                                                                                \
            draw_call;                                                                   \
            (shade) -= (shade_step);                                                     \
            if ((shade) < 0)                                                             \
            {                                                                            \
                (shade) = 0;                                                             \
            }                                                                            \
        }                                                                                \
                                                                                         \
        PopMatrix();                                                                     \
        if (--(timer) == 0)                                                              \
        {                                                                                \
            (step) += 1;                                                                 \
        }                                                                                \
    }

/**
 * @brief Define a model drop update: move along Z, draw, fade and count down.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer Frames left before advancing the step.
 * @param rotation The model's rotation.
 * @param position The model's position, clamped to WMAP_DROP_MIN_Z.
 * @param shade The model's shade global, clamped to zero after drawing.
 * @param resource Resource table containing the model at index zero.
 * @param z_delta Signed Z movement per frame; effect 07 also uses a positive value.
 * @param shade_step Amount subtracted from the shade each frame.
 * @note The timer keeps running after the model has faded out.
 */
#define WMAP_STEP_DROP_UPDATE(name, step, timer, rotation, position, shade, resource, z_delta, shade_step) \
    WMAP_STEP_DROP_UPDATE_WITH_DRAW(name, step, timer, rotation, position, shade,        \
        z_delta, shade_step,                                                             \
        wmap_draw_model_default((resource), 0, WMAP_DROP_OT_INDEX, WMAP_DROP_TPAGE,      \
            WMAP_DROP_CLUT, WMAP_DROP_BLEND_MODE, (shade)))

/**
 * @brief Define a model drop update using map rotation and the standard draw settings.
 * @param name Step function name.
 * @param step The sequence's step global.
 * @param timer Frames left before advancing the step.
 * @param rotation Rotation passed to the map renderer.
 * @param position The model's position, clamped to WMAP_DROP_MIN_Z.
 * @param shade The model's shade global, clamped to zero after drawing.
 * @param resource Resource table containing the model at index zero.
 * @param z_delta Signed Z movement per frame.
 * @param shade_step Amount subtracted from the shade each frame.
 */
#define WMAP_STEP_MAP_DROP_UPDATE(name, step, timer, rotation, position, shade, resource, z_delta, shade_step) \
    void name(void)                                                                      \
    {                                                                                    \
        s32 depth;                                                                       \
                                                                                         \
        depth = (position).vz + (z_delta);                                               \
        (position).vz = depth;                                                           \
        if (depth < WMAP_DROP_MIN_Z)                                                     \
        {                                                                                \
            (position).vz = WMAP_DROP_MIN_Z;                                             \
        }                                                                                \
                                                                                         \
        PushMatrix();                                                                    \
        wmap_set_map_rotation(&(rotation));                                              \
                                                                                         \
        if ((shade) != 0)                                                                \
        {                                                                                \
            wmap_draw_model((resource), 0, WMAP_DROP_OT_INDEX, WMAP_DROP_TPAGE,          \
                WMAP_DROP_CLUT, WMAP_DROP_BLEND_MODE, (shade), 0, 0, -1);                \
            (shade) -= (shade_step);                                                     \
            if ((shade) < 0)                                                             \
            {                                                                            \
                (shade) = 0;                                                             \
            }                                                                            \
        }                                                                                \
                                                                                         \
        PopMatrix();                                                                     \
        if (--(timer) == 0)                                                              \
        {                                                                                \
            (step) += 1;                                                                 \
        }                                                                                \
    }

/**
 * @brief Define the last step of a blocking sequence: clear the flag and finish.
 * @param name Step function name.
 * @param step The sequence's step global.
 */
#define WMAP_STEP_FINISH_BLOCKING(name, step)                                            \
    void name(void)                                                                      \
    {                                                                                    \
        g_wmap_sequence_busy = 0;                                                        \
        (step) += 1;                                                                     \
    }

/**
 * @brief Define a timeline's last step: clear g_wmap_sequence_busy, mark the focus cell and end.
 * @param name Step function name.
 * @param step The timeline's step global.
 * @param cells Land cell grid.
 * @param cell_x Cell x (first index into @p cells).
 * @param cell_y Cell y (second index into @p cells).
 * @param cell_value Value stored in the cell, with bit 0x100 set.
 */
#define WMAP_STEP_FINISH_MARK_CELL(name, step, cells, cell_x, cell_y, cell_value)                                                                              \
    void name(void)                                                                                                                                            \
    {                                                                                                                                                          \
        g_wmap_sequence_busy = 0;                                                                                                                              \
        cells[cell_x][cell_y].land_id = cell_value | 0x100;                                                                                                    \
        step += 1;                                                                                                                                             \
    }

#endif
