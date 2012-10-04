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
#ifndef drunkard_H
#define drunkard_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

/******************************************************************************\
Drunkard.
\******************************************************************************/

struct drunkard;

struct drunkard *drunkard_create(unsigned *tiles, unsigned w, unsigned h);
void drunkard_destroy(struct drunkard *drunk);

/* Core functions. */

bool drunkard_is_opened(struct drunkard *drunk, int x, int y);
void drunkard_set_open_threshold(struct drunkard *drunk, unsigned threshold);
void drunkard_mark(struct drunkard *drunk, int x, int y, unsigned tile);
void drunkard_flush_marks(struct drunkard *drunk);
void drunkard_push_state(struct drunkard *drunk);
void drunkard_pop_state(struct drunkard *drunk);
bool drunkard_is_stack_full(struct drunkard *drunk);

/* Query the drunkard. */

unsigned drunkard_count_opened(struct drunkard *drunk);
double drunkard_percent_opened(struct drunkard *drunk);
void drunkard_random_opened(struct drunkard *drunk, unsigned *x, unsigned *y);

int drunkard_get_x(struct drunkard *drunk);
int drunkard_get_y(struct drunkard *drunk);
int drunkard_get_target_x(struct drunkard *drunk);
int drunkard_get_target_y(struct drunkard *drunk);
int drunkard_get_dx_to_target(struct drunkard *drunk);
int drunkard_get_dy_to_target(struct drunkard *drunk);

/* RNG */

unsigned drunkard_get_seed(struct drunkard *drunk);
void drunkard_seed(struct drunkard *drunk, unsigned s);
double drunkard_rng_uniform(struct drunkard *drunk);
double drunkard_rng_under(struct drunkard *drunk, unsigned limit);
int drunkard_rng_range(struct drunkard *drunk, int low, int high);
bool drunkard_rng_chance(struct drunkard *drunk, double d);

/******************************************************************************\
Start functions.
\******************************************************************************/

void drunkard_start_fixed(struct drunkard *drunk, int x, int y);

void drunkard_start_random(struct drunkard *drunk);
void drunkard_start_random_west(struct drunkard *drunk);
void drunkard_start_random_east(struct drunkard *drunk);
void drunkard_start_random_north(struct drunkard *drunk);
void drunkard_start_random_south(struct drunkard *drunk);
void drunkard_start_random_west_edge(struct drunkard *drunk);
void drunkard_start_random_east_edge(struct drunkard *drunk);
void drunkard_start_random_north_edge(struct drunkard *drunk);
void drunkard_start_random_south_edge(struct drunkard *drunk);
void drunkard_start_random_westeast_edge(struct drunkard *drunk);
void drunkard_start_random_northsouth_edge(struct drunkard *drunk);
void drunkard_start_random_edge(struct drunkard *drunk);
void drunkard_start_random_opened(struct drunkard *drunk);

/******************************************************************************\
Targetting Functions.
\******************************************************************************/

void drunkard_target_fixed(struct drunkard *drunk, int x, int y);

void drunkard_target_random_west_edge(struct drunkard *drunk);
void drunkard_target_random_east_edge(struct drunkard *drunk);
void drunkard_target_random_north_edge(struct drunkard *drunk);
void drunkard_target_random_south_edge(struct drunkard *drunk);
void drunkard_target_random_westeast_edge(struct drunkard *drunk);
void drunkard_target_random_northsouth_edge(struct drunkard *drunk);
void drunkard_target_random_edge(struct drunkard *drunk);
void drunkard_target_random_opened(struct drunkard *drunk);

/******************************************************************************\
Mark functions.
\******************************************************************************/

void drunkard_mark_all(struct drunkard *drunk, unsigned tile);
void drunkard_mark_1(struct drunkard *drunk, unsigned tile);
void drunkard_mark_plus(struct drunkard *drunk, unsigned tile);
void drunkard_mark_x(struct drunkard *drunk, unsigned tile);
void drunkard_mark_rect(struct drunkard *drunk, int hw, int hh, unsigned tile);
void drunkard_mark_circle(struct drunkard *drunk, int r, unsigned tile);

/******************************************************************************\
Step functions.
\******************************************************************************/

void drunkard_step_by(struct drunkard *drunk, int dx, int dy);
void drunkard_step_random(struct drunkard *drunk);
void drunkard_step_to_target(struct drunkard *drunk, double weight);

void drunkard_line_path_to_target(struct drunkard *drunk);
void drunkard_tunnel_path_to_target(struct drunkard *drunk);
bool drunkard_walk_path(struct drunkard *drunk);
void drunkard_cancel_path(struct drunkard *drunk);

/******************************************************************************\
Checking functions.
\******************************************************************************/

bool drunkard_is_on_opened(struct drunkard *drunk);
bool drunkard_is_opened_on_rect(struct drunkard *drunk, unsigned hw, unsigned hh);
bool drunkard_is_opened_on_circle(struct drunkard *drunk, unsigned r);
bool drunkard_is_on_target(struct drunkard *drunk);
bool drunkard_is_on_fixed(struct drunkard *drunk, int x, int y);
bool drunkard_is_on_fixed_x(struct drunkard *drunk, int x);
bool drunkard_is_on_fixed_y(struct drunkard *drunk, int y);

#if defined(__cplusplus)
}
#endif


#endif
