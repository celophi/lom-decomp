/**
 * @file field_path_interpolation.c
 * @brief Closed B-spline paths for path-mode field effects.
 *
 * field_init_path scatters control points on a ring around an effect and sets
 * up a uniform, closed spline of FIELD_PATH_DEGREE through them;
 * field_update_path_position then evaluates the spline at the effect's path
 * time. Weights and knots use 20.12 fixed point (ONE is 1.0), control points
 * are whole field units (positions >> 8).
 */

#include "common.h"
#include "field_path_interpolation.h"
#include "sdk/libgte.h"
#include "sdk/rand.h"

/** @brief Control point slots per path group (FIELD_PATH_POINT_COUNT are used). */
#define FIELD_PATH_POINT_CAPACITY 11

/** @brief Control points field_init_path generates. */
#define FIELD_PATH_POINT_COUNT 10

/** @brief Spline degree field_init_path selects (cubic). */
#define FIELD_PATH_DEGREE 3

/** @brief Path time steps between two control points. */
#define FIELD_PATH_SEGMENT_STEPS 20

/** @brief Ored into a 15-bit rand() value so a random radius is at least half the requested one. */
#define FIELD_PATH_MIN_RADIUS_BIT 0x4000

/** @brief Control points of one path, X and Z in whole field units. */
typedef struct
{
    s16 x[FIELD_PATH_POINT_CAPACITY];
    s16 z[FIELD_PATH_POINT_CAPACITY];
} FieldPathGroup;

extern s32 g_field_path_segment_steps;
extern FieldPathGroup g_field_path_groups[FIELD_PATH_GROUP_COUNT];

/** @brief Spline order (degree + 1). */
extern s32 g_field_path_order;

/** @brief Path time steps per loop; path time wraps here. */
extern s32 g_field_path_length;

/** @brief Control points the evaluation wraps around. */
extern s32 g_field_path_point_count;

/** @brief Spline order / 2. */
extern s32 g_field_path_half_order;

/** @brief Basis functions of the unwrapped spline (points + degree). */
extern s32 g_field_path_basis_count;

/** @brief Knots of the unwrapped spline (points + 2 * degree). */
extern s32 g_field_path_knot_count;

extern s32 g_field_path_degree;

/** @brief Control points field_init_path generated. */
extern s32 g_field_path_generated_count;

static void field_evaluate_path(s32 time, FieldMotionRecord* record, s32 group);
static void field_build_path_knots(s32* knots);
static void field_fold_path_weights(s32 (*basis)[2], s32* weights);
static void field_compute_path_basis(s32 order, s32 parameter, s32* span, s32* knots, s32 (*basis)[2]);

/**
 * @brief Wrap an effect's path time into the path length and move the effect to that point.
 * @param time Path time of the effect, wrapped in place.
 * @param record Effect whose x and z are set.
 * @param group Path group the effect follows.
 */
void field_update_path_position(u8* time, FieldMotionRecord* record, s32 group)
{
    if (*time >= g_field_path_length)
    {
        *time -= g_field_path_length;
    }
    field_evaluate_path(*time, record, group);
}

/**
 * @brief Scatter a path group's control points on a ring around an effect and set up the spline.
 * @param center Effect whose x and z are the ring center.
 * @param radius Ring radius in whole field units.
 * @param randomize Nonzero to give each point a random radius from radius / 2 to radius.
 * @param group Path group to fill.
 */
void field_init_path(FieldMotionRecord* center, s32 radius, s32 randomize, s32 group)
{
    s32 i;
    s32 angle;
    s32 point_radius;

    g_field_path_degree = FIELD_PATH_DEGREE;
    g_field_path_segment_steps = FIELD_PATH_SEGMENT_STEPS;
    g_field_path_generated_count = FIELD_PATH_POINT_COUNT;
    for (i = 0; i < FIELD_PATH_POINT_COUNT; i++)
    {
        angle = rand() >> 3;
        if (randomize)
        {
            point_radius = ((rand() | FIELD_PATH_MIN_RADIUS_BIT) * radius) >> 15;
        }
        else
        {
            point_radius = radius;
        }
        g_field_path_groups[group].x[i] = ((rcos(angle) * point_radius) >> 12) + (center->x >> 8);
        g_field_path_groups[group].z[i] = ((rsin(angle) * point_radius) >> 12) + (center->z >> 8);
    }
    g_field_path_point_count = g_field_path_generated_count;
    g_field_path_basis_count = g_field_path_generated_count + g_field_path_degree;
    g_field_path_order = g_field_path_degree + 1;
    g_field_path_half_order = g_field_path_order >> 1;
    g_field_path_knot_count = g_field_path_generated_count + g_field_path_degree * 2;
    g_field_path_length = g_field_path_segment_steps * g_field_path_generated_count + 1;
}

/**
 * @brief Evaluate a path group's spline at a path time.
 * @param time Path time, 0 to g_field_path_length - 1.
 * @param record Effect whose x and z receive the point.
 * @param group Path group to evaluate.
 * @note basis holds 11 rows but the recurrence clears g_field_path_basis_count + 1 of
 *       them; the extra rows run into weights, which is only filled after they are read.
 */
static void field_evaluate_path(s32 time, FieldMotionRecord* record, s32 group)
{
    s32 basis[11][2];
    s32 weights[12];
    s32 knots[34];
    s32 span;
    s32 first;
    s32 last;
    s32 i;
    s32 x;
    s32 z;

    field_build_path_knots(knots);
    field_compute_path_basis(g_field_path_order, (time << 12) / g_field_path_segment_steps, &span, knots, basis);
    field_fold_path_weights(basis, weights);
    last = span - g_field_path_half_order + 1;
    first = last - g_field_path_degree;
    if (first < 0 || last > g_field_path_point_count - 1)
    {
        first = 0;
        last = g_field_path_point_count - 1;
    }
    x = 0;
    z = 0;
    for (i = first; i <= last; i++)
    {
        x += (g_field_path_groups[group].x[i] * weights[i]) >> 12;
        z += (g_field_path_groups[group].z[i] * weights[i]) >> 12;
    }
    record->x = x << 8;
    record->z = z << 8;
}

/**
 * @brief Build the uniform knot vector, starting at 1 - order so the first span begins at 0.
 * @param knots Receives g_field_path_knot_count knots.
 */
static void field_build_path_knots(s32* knots)
{
    s32 i;

    for (i = 0; i < g_field_path_knot_count; i++)
    {
        knots[i] = (i + 1 - g_field_path_order) << 12;
    }
}

/**
 * @brief Fold the unwrapped basis weights onto the control points of the closed path.
 * @param basis Basis weights from field_compute_path_basis; the last level's column is used.
 * @param weights Receives one weight per control point.
 */
static void field_fold_path_weights(s32 (*basis)[2], s32* weights)
{
    s32 i;

    for (i = 0; i < g_field_path_half_order; i++)
    {
        weights[i] = basis[i + g_field_path_half_order - 1][(g_field_path_order - 1) & 1] +
                     basis[i + g_field_path_point_count + g_field_path_half_order - 1][(g_field_path_order - 1) & 1];
    }
    for (i = 0; i < g_field_path_point_count - g_field_path_degree; i++)
    {
        weights[i + g_field_path_half_order] = basis[i + g_field_path_degree][(g_field_path_order - 1) & 1];
    }
    for (i = 0; i < g_field_path_half_order - 1; i++)
    {
        weights[i + g_field_path_point_count - g_field_path_half_order + 1] =
            basis[i + g_field_path_point_count][(g_field_path_order - 1) & 1] + basis[i][(g_field_path_order - 1) & 1];
    }
}

/**
 * @brief Evaluate the B-spline basis functions at a parameter (Cox-de Boor recurrence).
 * @param order Spline order; levels 1 to order - 1 are computed.
 * @param parameter Spline parameter in 20.12 knot units.
 * @param span Receives the knot span that contains @p parameter.
 * @param knots Knot vector from field_build_path_knots.
 * @param basis Weights per basis function; level n is kept in column n & 1.
 */
static void field_compute_path_basis(s32 order, s32 parameter, s32* span, s32* knots, s32 (*basis)[2])
{
    s32 i;
    s32 level;
    s32 left;
    s32 right;

    for (i = 0; i < g_field_path_basis_count + 1; i++)
    {
        basis[i][0] = 0;
    }
    for (i = 0; i < g_field_path_basis_count; i++)
    {
        if (parameter >= knots[i] && parameter < knots[i + 1])
        {
            basis[i][0] = ONE;
            *span = i;
        }
    }
    if (parameter >= knots[g_field_path_basis_count - 1] && parameter <= knots[g_field_path_basis_count] + 1)
    {
        basis[g_field_path_basis_count - 1][0] = ONE;
        *span = g_field_path_basis_count - 1;
    }
    for (level = 1; level < order; level++)
    {
        for (i = 0; i < g_field_path_basis_count + 1; i++)
        {
            basis[i][level & 1] = 0;
        }
        for (i = *span - level; i <= *span; i++)
        {
            left = right = 0;
            if (knots[i + 1] != knots[i + level + 1])
            {
                right = ((knots[i + level + 1] - parameter) * basis[i + 1][(level - 1) & 1]) / (knots[i + level + 1] - knots[i + 1]);
            }
            if (knots[i] != knots[i + level])
            {
                left = ((parameter - knots[i]) * basis[i][(level - 1) & 1]) / (knots[i + level] - knots[i]);
            }
            basis[i][level & 1] = right + left;
        }
    }
}
