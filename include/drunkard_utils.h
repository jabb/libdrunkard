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
#ifndef WALKER_UTILS_H
#define WALKER_UTILS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

#include "drunkard.h"

/******************************************************************************\
\******************************************************************************/

typedef bool drunkard_pattern_func(struct drunkard *drunk, void *args);

struct drunkard_pattern
{
    struct drunkard_pattern *prev;
    drunkard_pattern_func *pattern_func;
    void *args;
    unsigned weight;
};

struct drunkard_plans
{
    struct drunkard_pattern *patterns;
    double min_percent_open;
    unsigned max_iterations;

    unsigned first_open_tile;
    unsigned default_wall_tile;
    unsigned default_floor_tile;
    bool border;
};

struct drunkard_plans drunkard_make_plans(void);
void drunkard_unmake_plans(struct drunkard_plans *plans);

bool drunkard_plans_add_cave(
    struct drunkard_plans *plans,
    unsigned weight,
    unsigned floor_tile,
    double wavey);

bool drunkard_plans_add_room_and_corridor(
    struct drunkard_plans *plans,
    unsigned weight,
    unsigned floor_tile,
    unsigned minsize, unsigned maxsize);

void drunkard_carve_plans(struct drunkard *drunk, struct drunkard_plans *plans);

#if defined(__cplusplus)
}
#endif

#endif
