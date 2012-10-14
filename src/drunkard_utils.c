/* Copyright (c) 2012, Michael Patraw
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of Michael Patraw may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Michael Patraw ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Michael Patraw BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "drunkard_utils.h"

 #include <stdlib.h>
 #include <string.h>

/******************************************************************************\
Pattern carving functions.
\******************************************************************************/

void drunkard_pattern_destroy(struct drunkard_pattern *patt)
{
    if (patt)
    {
        if (patt->args)
            free(patt->args);
        free(patt);
    }
}

static struct drunkard_pattern *drunkard_pattern_create(
    struct drunkard_pattern *prev,
    drunkard_pattern_func *pattern_func,
    unsigned args_size,
    unsigned weight)
{
    struct drunkard_pattern *patt = malloc(sizeof *patt);
    if (!patt)
        goto failed;

    patt->prev = prev;
    patt->pattern_func = pattern_func;
    patt->args = malloc(args_size);
    if (!patt->args)
        goto failed;
    patt->weight = weight;

    return patt;

failed:
    drunkard_pattern_destroy(patt);
    return NULL;
}

struct drunkard_generic_args
{
    unsigned floor_tile;
    unsigned wall_tile;
    unsigned extra_tile;
    unsigned min_width, min_height;
    unsigned max_width, max_height;
    double randomness;
};

static int carve_shrinking_square(struct drunkard *drunk, int min, int max, unsigned tile)
{
    bool has_opened;

    do {
        has_opened = drunkard_is_opened_on_rect(drunk, max + 1, max + 1);
        max--;
    } while (has_opened && max >= min);

    if (!has_opened)
    {
        drunkard_mark_rect(drunk, max + 1, max + 1, tile);
        return max + 1;
    }
    return 0;
}
#if 0
static int carve_shrinking_circle(struct drunkard *drunk, int min, int max, unsigned tile)
{
    bool has_opened;

    do {
        has_opened = drunkard_is_opened_on_circle(drunk, max + 1);
        max--;
    } while (has_opened && max >= min);

    if (!has_opened)
    {
        drunkard_mark_circle(drunk, max + 1, tile);
        /* +1 for the max--; in the loop */
        return max + 1;
    }
    return 0;
}
#endif
static void carve_cave(struct drunkard *drunk, void *args)
{
    struct drunkard_generic_args *pargs = args;

    drunkard_start_random(drunk);
    drunkard_target_random_opened(drunk);

    while (!drunkard_is_on_opened(drunk))
    {
        drunkard_mark_plus(drunk, pargs->floor_tile);
        drunkard_step_to_target(drunk, pargs->randomness);
    }

    drunkard_flush_marks(drunk);
}

static void carve_room_then_corridor(struct drunkard *drunk, void *args)
{
    struct drunkard_generic_args *pargs = args;

    drunkard_start_random(drunk);
    drunkard_target_random_opened(drunk);

    int marked = carve_shrinking_square(drunk,
        pargs->min_width, pargs->max_width, pargs->floor_tile);

    if (marked)
    {
        drunkard_tunnel_path_to_target(drunk);
        while (drunkard_walk_path(drunk) && !drunkard_is_on_opened(drunk))
        {
            drunkard_mark_1(drunk, pargs->floor_tile);
        }
    }

    drunkard_flush_marks(drunk);
}

/******************************************************************************\
Exposed functions.
\******************************************************************************/

struct drunkard_plans drunkard_make_plans(void)
{
    struct drunkard_plans plans;
    memset(&plans, 0, sizeof plans);
    /* Acceptable defaults, methinks. */
    plans.min_percent_open = 1.0;
    plans.max_iterations = ~((unsigned)0);
    plans.first_open_tile = 1;
    plans.default_wall_tile = 0;
    plans.default_floor_tile = 1;
    return plans;
}

void drunkard_unmake_plans(struct drunkard_plans *plans)
{
    struct drunkard_pattern *prev, *patt = plans->patterns;

    if (!patt)
        return;

    while (patt)
    {
        prev = patt->prev;
        drunkard_pattern_destroy(patt);
        patt = prev;
    }
}

bool drunkard_plans_add_cave(
    struct drunkard_plans *plans,
    unsigned weight,
    unsigned floor_tile,
    double wavey)
{
    struct drunkard_generic_args *args;
    struct drunkard_pattern *patt = drunkard_pattern_create(
        plans->patterns,
        carve_cave,
        sizeof(struct drunkard_generic_args),
        weight);
    if (!patt)
        goto failed;

    plans->patterns = patt;
    args = patt->args;
    args->floor_tile = floor_tile;
    /* Waviness can be from 0.0 to 1.0, but libdrunkard's argument is on the
     * scale of 0.5 to 1.0. Here we translate it.
     */
    args->randomness = 0.5 + (wavey * 0.5);

    return true;

failed:
    return false;
}

bool drunkard_plans_add_room_and_corridor(
    struct drunkard_plans *plans,
    unsigned weight,
    unsigned floor_tile,
    unsigned minsize, unsigned maxsize)
{
    struct drunkard_generic_args *args;
    struct drunkard_pattern *patt = drunkard_pattern_create(
        plans->patterns,
        carve_room_then_corridor,
        sizeof(struct drunkard_generic_args),
        weight);
    if (!patt)
        goto failed;

    plans->patterns = patt;
    args = patt->args;
    args->floor_tile = floor_tile;
    args->min_width = minsize;
    args->min_height = minsize;
    args->max_width = maxsize;
    args->max_height = maxsize;

    return true;

failed:
    return false;
}

void drunkard_carve_plans(struct drunkard *drunk, struct drunkard_plans *plans)
{
    unsigned max_weight = 0, i = 0;
    int r;

    struct drunkard_pattern *patt = plans->patterns;

    if (!patt)
        return;

    while (patt)
    {
        max_weight += patt->weight;
        patt = patt->prev;
    }

    drunkard_set_open_threshold(drunk, plans->first_open_tile);
    drunkard_set_border(drunk, true);

    drunkard_start_random(drunk);
    drunkard_mark_1(drunk, plans->default_floor_tile);
    drunkard_flush_marks(drunk);

    while (drunkard_percent_opened(drunk) < plans->min_percent_open &&
        i++ < plans->max_iterations)
    {
        r = drunkard_rng_range(drunk, 1, max_weight);

        patt = plans->patterns;
        while (r - (int)patt->weight > 0)
        {
            r -= patt->weight;
            patt = patt->prev;
        }

        patt->pattern_func(drunk, patt->args);
    }
}
