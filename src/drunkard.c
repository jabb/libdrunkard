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

/* Design Note:
 * init* must be followed by an uninit* when done with the object.
 * new* objects can be free'd with a simple call to free()
 * create* objects must be cleaned up with destroy*
 * make* puts the object on the stack, no dynamic allocation.
 */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "drunkard.h"

/******************************************************************************\
RNG.
\******************************************************************************/

/* Medium speed. High memory. Period ~= 2^131104. */
enum {CMWC_K = 4096};
struct rng_state {
    uint32_t q[CMWC_K];
    uint32_t c;
    uint32_t i;
};

void rng_seed(struct rng_state *st, uint32_t seed)
{
    srand(seed);
    for (int i = 0; i < CMWC_K; ++i) {
        st->q[i] = rand();
    }
    st->c = (seed < 809430660) ? seed : 809430660 - 1;
    st->i = CMWC_K - 1;
}

uint32_t rng_u32(struct rng_state *st)
{
    uint32_t a = 18782;
    uint32_t r = 0xfffffffe;

    st->i = (st->i + 1) & (CMWC_K - 1);

    uint32_t q = st->q[st->i];
    uint32_t qh = q >> 16;
    uint32_t ql = q & 0xffff;

    uint32_t tl = a * q + st->c;
    uint32_t th = (a * qh + (((st->c & 0xffff) + a * ql) >> 16) + (st->c >> 16)) >> 16;


    st->c = th;
    uint32_t x = tl + th;
    if (x < st->c) {
        x++;
        st->c++;
    }

    return st->q[st->i] = r - x;
}

double rng_uniform(struct rng_state *st)
{
    return rng_u32(st) * 2.3283064365386963e-10;
}

double rng_under(struct rng_state *st, int32_t max)
{
    return rng_uniform(st) * max;
}

double rng_between(struct rng_state *st, int32_t min, int32_t max)
{
    return rng_under(st, max - min) + min;
}

int32_t rng_range(struct rng_state *st, int32_t min, int32_t max)
{
    return floor(rng_between(st, min, max + 1));
}

bool rng_chance(struct rng_state *st, double ch)
{
    return rng_uniform(st) < ch;
}

/******************************************************************************\
Point set.
\******************************************************************************/

struct point { int x, y; };

struct point make_point(int x, int y)
{
    struct point p;
    p.x = x;
    p.y = y;
    return p;
}

#define INDEX1(arr, i) ((arr)[(i)])
#define INDEX2(arr, x, y, w) INDEX1(arr, (y) * (w) + (x))

struct pointset
{
    struct point *arr;
    unsigned length;

    bool *map;
    unsigned width, height;
};

#define POINTSET_FOR(ps, i, p) \
    for ((i) = 0, (p) = &(ps)->arr[(i)]; \
         (i) < (ps)->length; \
         (i)++, (p)++)

bool pointset_init(struct pointset *ps, unsigned width, unsigned height)
{
    memset(ps, 0, sizeof *ps);

    ps->arr = malloc(sizeof *ps->arr * width * height);
    if (!ps->arr)
        goto alloc_failure;
    memset(ps->arr, 0, sizeof *ps->arr * width * height);

    ps->map = malloc(sizeof *ps->map * width * height);
    if (!ps->map)
        goto alloc_failure;
    memset(ps->map, 0, sizeof *ps->map * width * height);

    ps->length = 0;
    ps->width = width;
    ps->height = height;

    return true;

alloc_failure:
    if (ps->map)
        free(ps->map);
    if (ps->arr)
        free(ps->arr);
    return false;
}

void pointset_uninit(struct pointset *ps)
{
    free(ps->arr);
    free(ps->map);
}

bool pointset_has(struct pointset *ps, struct point p)
{
    if (INDEX2(ps->map, p.x, p.y, ps->width))
        return true;
    return false;
}

bool pointset_add(struct pointset *ps, struct point p)
{
    if (!pointset_has(ps, p))
    {
        INDEX2(ps->map, p.x, p.y, ps->width) = true;
        ps->arr[ps->length] = p;
        ps->length++;
        return true;
    }

    return false;
}

bool pointset_rem(struct pointset *ps, struct point p)
{
    if (pointset_has(ps, p))
    {
        INDEX2(ps->map, p.x, p.y, ps->width) = false;

        unsigned i;
        for (i = 0; i < ps->length; ++i)
            if (ps->arr[i].x == p.x && ps->arr[i].y == p.y)
                break;

        memmove(&ps->arr[i], &ps->arr[i + 1], (ps->length - i) * sizeof *ps->arr);
        ps->length--;
        return true;
    }

    return false;
}

struct point pointset_random(struct pointset *ps, struct rng_state *rng)
{
    return ps->arr[rng_range(rng, 0, ps->length - 1)];
}

void pointset_clear(struct pointset *ps)
{
    ps->length = 0;
    memset(ps->map, 0, sizeof *ps->map * ps->width * ps->height);
}

/******************************************************************************\
Drunkard.
\******************************************************************************/

#define TILE_AT(w, x, y) (INDEX2((w)->tiles, x, y, (w)->width))
#define IN_BOUNDS(w, x, y) \
    ((x) >= 0 && (y) >= 0 && (x) < (int)(w)->width && (y) < (int)(w)->height)

struct drunkard
{
    struct pointset openedset;
    struct pointset markedset;
    struct rng_state rng;
    unsigned seed;

    unsigned *tiles;
    unsigned width, height;
    unsigned open_threshold;

    int x, y;
    int target_x, target_y;

    unsigned char path_data[256];
    bool (*pathing_function) (struct drunkard *);
};

struct drunkard *drunkard_create(unsigned *tiles, unsigned w, unsigned h)
{
    struct drunkard *drunk = malloc(sizeof *drunk);
    if (!drunk)
        goto alloc_failure;

    if (!pointset_init(&drunk->markedset, w, h))
        goto markedset_init_failure;

    if (!pointset_init(&drunk->openedset, w, h))
        goto openedset_init_failure;

    drunk->seed = time(NULL);
    rng_seed(&drunk->rng, drunk->seed);

    drunk->tiles = tiles;
    drunk->width = w;
    drunk->height = h;
    drunk->open_threshold = 1;

    drunk->x = -1;
    drunk->y = -1;
    drunk->target_x = -1;
    drunk->target_y = -1;

    drunk->pathing_function = NULL;

    return drunk;

openedset_init_failure:
    pointset_uninit(&drunk->markedset);
markedset_init_failure:
    free(drunk);
alloc_failure:
    return NULL;
}

void drunkard_destroy(struct drunkard *drunk)
{
    pointset_uninit(&drunk->markedset);
    pointset_uninit(&drunk->openedset);
    free(drunk);
}

bool drunkard_is_opened(struct drunkard *drunk, int x, int y)
{
    return pointset_has(&drunk->openedset, make_point(x, y));
}

void drunkard_set_open_threshold(struct drunkard *drunk, unsigned threshold)
{
    drunk->open_threshold = threshold;
}

void drunkard_mark(struct drunkard *drunk, int x, int y, unsigned tile)
{
    if (IN_BOUNDS(drunk, x, y))
    {
        if (tile >= drunk->open_threshold)
        {
            pointset_add(&drunk->markedset, make_point(x, y));
        }
        else
        {
            pointset_rem(&drunk->markedset, make_point(x, y));
            pointset_rem(&drunk->openedset, make_point(x, y));
        }
        TILE_AT(drunk, x, y) = tile;
    }
}

void drunkard_flush_marks(struct drunkard *drunk)
{
    unsigned i;
    struct point *pi;

    POINTSET_FOR(&drunk->markedset, i, pi)
        pointset_add(&drunk->openedset, *pi);

    pointset_clear(&drunk->markedset);
}

unsigned drunkard_count_opened(struct drunkard *drunk)
{
    return drunk->openedset.length;
}

double drunkard_percent_opened(struct drunkard *drunk)
{
    unsigned size = drunk->width * drunk->height;
    return (double)drunkard_count_opened(drunk) / (double)size;
}

void drunkard_random_opened(struct drunkard *drunk, unsigned *x, unsigned *y)
{
    struct point p = pointset_random(&drunk->openedset, &drunk->rng);
    *x = p.x;
    *y = p.y;
}

int drunkard_get_x(struct drunkard *drunk)
{
    return drunk->x;
}

int drunkard_get_y(struct drunkard *drunk)
{
    return drunk->y;
}

int drunkard_get_target_x(struct drunkard *drunk)
{
    return drunk->target_x;
}

int drunkard_get_target_y(struct drunkard *drunk)
{
    return drunk->target_y;
}

int drunkard_get_dx_to_target(struct drunkard *drunk)
{
    return drunkard_get_target_x(drunk) - drunkard_get_x(drunk);
}

int drunkard_get_dy_to_target(struct drunkard *drunk)
{
    return drunkard_get_target_y(drunk) - drunkard_get_y(drunk);
}

unsigned drunkard_get_seed(struct drunkard *drunk)
{
    return drunk->seed;
}

void drunkard_seed(struct drunkard *drunk, unsigned s)
{
    drunk->seed = s;
    rng_seed(&drunk->rng, s);
}

double drunkard_rng_uniform(struct drunkard *drunk)
{
    return rng_uniform(&drunk->rng);
}

double drunkard_rng_under(struct drunkard *drunk, unsigned limit)
{
    return rng_under(&drunk->rng, limit);
}

int drunkard_rng_range(struct drunkard *drunk, int low, int high)
{
    return rng_range(&drunk->rng, low, high);
}

bool drunkard_rng_chance(struct drunkard *drunk, double d)
{
    if (rng_uniform(&drunk->rng) < d)
        return true;
    return false;
}

/******************************************************************************\
Start functions.
\******************************************************************************/

void drunkard_start_fixed(struct drunkard *drunk, int x, int y)
{
    drunk->x = x;
    drunk->y = y;
}

void drunkard_start_random(struct drunkard *drunk)
{
    drunk->x = drunkard_rng_range(drunk, 0, drunk->width - 1);
    drunk->y = drunkard_rng_range(drunk, 0, drunk->height - 1);
}

void drunkard_start_random_west(struct drunkard *drunk)
{
    drunk->x = drunkard_rng_range(drunk, 0, drunk->width / 2);
    drunk->y = drunkard_rng_range(drunk, 0, drunk->height - 1);
}

void drunkard_start_random_east(struct drunkard *drunk)
{
    drunk->x = drunkard_rng_range(drunk, drunk->width / 2, drunk->width - 1);
    drunk->y = drunkard_rng_range(drunk, 0, drunk->height - 1);
}

void drunkard_start_random_north(struct drunkard *drunk)
{
    drunk->x = drunkard_rng_range(drunk, 0, drunk->width - 1);
    drunk->y = drunkard_rng_range(drunk, 0, drunk->height / 2);
}

void drunkard_start_random_south(struct drunkard *drunk)
{
    drunk->x = drunkard_rng_range(drunk, 0, drunk->width - 1);
    drunk->y = drunkard_rng_range(drunk, drunk->height / 2, drunk->height - 1);
}

void drunkard_start_random_west_edge(struct drunkard *drunk)
{
    drunk->x = 0;
    drunk->y = drunkard_rng_range(drunk, 0, drunk->height - 1);
}

void drunkard_start_random_east_edge(struct drunkard *drunk)
{
    drunk->x = drunk->width - 1;
    drunk->y = drunkard_rng_range(drunk, 0, drunk->height - 1);
}

void drunkard_start_random_north_edge(struct drunkard *drunk)
{
    drunk->x = drunkard_rng_range(drunk, 0, drunk->width - 1);
    drunk->y = 0;
}

void drunkard_start_random_south_edge(struct drunkard *drunk)
{
    drunk->x = drunkard_rng_range(drunk, 0, drunk->width - 1);
    drunk->y = drunk->height - 1;
}

void drunkard_start_random_westeast_edge(struct drunkard *drunk)
{
    if (drunkard_rng_chance(drunk, 0.5))
        drunkard_start_random_west_edge(drunk);
    else
        drunkard_start_random_east_edge(drunk);
}

void drunkard_start_random_northsouth_edge(struct drunkard *drunk)
{
    if (drunkard_rng_chance(drunk, 0.5))
        drunkard_start_random_north_edge(drunk);
    else
        drunkard_start_random_south_edge(drunk);
}

void drunkard_start_random_edge(struct drunkard *drunk)
{
    if (drunkard_rng_chance(drunk, 0.5))
        drunkard_start_random_westeast_edge(drunk);
    else
        drunkard_start_random_northsouth_edge(drunk);
}

void drunkard_start_random_opened(struct drunkard *drunk)
{
    struct point p = pointset_random(&drunk->openedset, &drunk->rng);
    drunk->x = p.x;
    drunk->y = p.y;
}

/******************************************************************************\
Targetting Functions.
\******************************************************************************/

void drunkard_target_fixed(struct drunkard *drunk, int x, int y)
{
    drunk->target_x = x;
    drunk->target_y = y;
}

void drunkard_target_random_west_edge(struct drunkard *drunk)
{
    drunk->target_x = 0;
    drunk->target_y = drunkard_rng_range(drunk, 0, drunk->height - 1);
}

void drunkard_target_random_east_edge(struct drunkard *drunk)
{
    drunk->target_x = drunk->width - 1;
    drunk->target_y = drunkard_rng_range(drunk, 0, drunk->height - 1);
}

void drunkard_target_random_north_edge(struct drunkard *drunk)
{
    drunk->target_x = drunkard_rng_range(drunk, 0, drunk->width - 1);
    drunk->target_y = 0;
}

void drunkard_target_random_south_edge(struct drunkard *drunk)
{
    drunk->target_x = drunkard_rng_range(drunk, 0, drunk->width - 1);
    drunk->target_y = drunk->height - 1;
}

void drunkard_target_random_westeast_edge(struct drunkard *drunk)
{
    if (drunkard_rng_chance(drunk, 0.5))
        drunkard_target_random_west_edge(drunk);
    else
        drunkard_target_random_east_edge(drunk);
}

void drunkard_target_random_northsouth_edge(struct drunkard *drunk)
{
    if (drunkard_rng_chance(drunk, 0.5))
        drunkard_target_random_north_edge(drunk);
    else
        drunkard_target_random_south_edge(drunk);
}

void drunkard_target_random_edge(struct drunkard *drunk)
{
    if (drunkard_rng_chance(drunk, 0.5))
        drunkard_target_random_westeast_edge(drunk);
    else
        drunkard_target_random_northsouth_edge(drunk);
}

void drunkard_target_random_opened(struct drunkard *drunk)
{
    struct point p = pointset_random(&drunk->openedset, &drunk->rng);
    drunk->target_x = p.x;
    drunk->target_y = p.y;
}

/******************************************************************************\
Mark functions.
\******************************************************************************/

void drunkard_mark_all(struct drunkard *drunk, unsigned tile)
{
    for (unsigned x = 0; x < drunk->width; ++x)
        for (unsigned y = 0; y < drunk->height; ++y)
            drunkard_mark(drunk, x, y, tile);
}

void drunkard_mark_1(struct drunkard *drunk, unsigned tile)
{
    drunkard_mark(drunk, drunk->x, drunk->y, tile);
}

void drunkard_mark_plus(struct drunkard *drunk, unsigned tile)
{
    drunkard_mark(drunk, drunk->x, drunk->y, tile);
    drunkard_mark(drunk, drunk->x - 1, drunk->y, tile);
    drunkard_mark(drunk, drunk->x, drunk->y - 1, tile);
    drunkard_mark(drunk, drunk->x, drunk->y + 1, tile);
    drunkard_mark(drunk, drunk->x + 1, drunk->y, tile);
}

void drunkard_mark_x(struct drunkard *drunk, unsigned tile)
{
    drunkard_mark(drunk, drunk->x, drunk->y, tile);
    drunkard_mark(drunk, drunk->x - 1, drunk->y - 1, tile);
    drunkard_mark(drunk, drunk->x - 1, drunk->y + 1, tile);
    drunkard_mark(drunk, drunk->x + 1, drunk->y - 1, tile);
    drunkard_mark(drunk, drunk->x + 1, drunk->y + 1, tile);
}

void drunkard_mark_rect(struct drunkard *drunk, int hw, int hh, unsigned tile)
{
    for (int x = ceil(drunk->x - hw); x <= floor(drunk->x + hw); ++x)
        for (int y = ceil(drunk->y - hh); y <= floor(drunk->y + hh); ++y)
            drunkard_mark(drunk, x, y, tile);
}

void drunkard_mark_circle(struct drunkard *drunk, int r, unsigned tile)
{
    for (int dx = -r; dx <= r; ++dx)
    {
        int h = floor(sqrt(r * r - dx * dx) - 0.0000001);
        for (int dy = -h; dy <= h; ++dy)
        {
            int x = drunkard_get_x(drunk) + dx;
            int y = drunkard_get_y(drunk) + dy;
            drunkard_mark(drunk, x, y, tile);
        }
    }
}

/******************************************************************************\
Step functions.
\******************************************************************************/

void drunkard_step_by(struct drunkard *drunk, int dx, int dy)
{
    bool in_bounds = IN_BOUNDS(drunk, drunk->x, drunk->y);
    bool target_in_bounds = IN_BOUNDS(drunk, drunk->x + dx, drunk->y + dy);
    if (target_in_bounds || (!in_bounds && !target_in_bounds))
    {
        drunk->x += dx;
        drunk->y += dy;
    }
}

void drunkard_step_random(struct drunkard *drunk)
{
    int dirs[9][2] = {
        {-1,  0},
        { 0, -1},
        { 0,  1},
        { 1,  0},
    };

    int *dir = dirs[drunkard_rng_range(drunk, 0, 3)];
    int dx = dir[0];
    int dy = dir[1];

    drunkard_step_by(drunk, dx, dy);
}

void drunkard_step_to_target(struct drunkard *drunk, double weight)
{
    int dx = drunk->target_x - drunk->x;
    int dy = drunk->target_y - drunk->y;

    if (dx >  1) dx = 1;
    if (dx < -1) dx = -1;
    if (dy >  1) dy = 1;
    if (dy < -1) dy = -1;

    double r = drunkard_rng_uniform(drunk);

    if (dx == 0 && dy == 0)
    {

    }
    else if (dx == 0 || dy == 0)
    {
        int *dpz, *dpnz;
        if (dx == 0)
        {
            dpz = &dx;
            dpnz = &dy;
        }
        else
        {
            dpz = &dy;
            dpnz = &dx;
        }

        if (r < 0.5)
        {
            if (r < 0.25)
            {
                *dpz = -1;
                *dpnz = 0;
            }
            else
            {
                *dpz = 1;
                *dpnz = 0;
            }
        }
        else if (r < 0.5 + (weight / 2))
        {
        }
        else
        {
            if (dx != 0)
                dx *= -1;
            if (dy != 0)
                dy *= -1;
        }
    }
    else
    {
        if (r < (1 - weight))
        {
            dx *= -1;
            dy *= -1;
        }

        if (drunkard_rng_chance(drunk, 0.5))
            dx = 0;
        else
            dy = 0;
    }

    drunkard_step_by(drunk, dx, dy);
}

struct line_path_struct
{
    int dx;
    int sx;
    int dy;
    int sy;
    int err;
    int e2;
};

static bool line_path(struct drunkard *drunk)
{
    struct line_path_struct *data = (void *)drunk->path_data;

    if (drunk->x == drunk->target_x && drunk->y == drunk->target_y)
        return false;

    data->e2 = data->err;

    if (data->e2 > -data->dx)
    {
        data->err -= data->dy;
        drunk->x += data->sx;
    }
    if (data->e2 < data->dy)
    {
        data->err += data->dx;
        drunk->y += data->sy;
    }

    return true;
}

void drunkard_line_path_to_target(struct drunkard *drunk)
{
    struct line_path_struct *data = (void *)drunk->path_data;

    data->dx = abs(drunk->target_x - drunk->x);
    data->sx = drunk->x < drunk->target_x ? 1 : -1;

    data->dy = abs(drunk->target_y - drunk->y);
    data->sy = drunk->y < drunk->target_y ? 1 : -1;

    data->err = (data->dx > data->dy ? data->dx : -data->dy) / 2;

    drunk->pathing_function = line_path;
}

enum {NONE, HORIZONTAL, VERTICAL};

struct tunnel_path_struct
{
    int first;
    int second;
};

static bool tunnel_path(struct drunkard *drunk)
{
    struct tunnel_path_struct *data = (void *)drunk->path_data;

    if (!data->first)
        return false;

    if (data->first == HORIZONTAL)
    {
        drunk->x += drunkard_get_dx_to_target(drunk) < 0 ? -1 : 1;

        if (drunkard_get_x(drunk) == drunkard_get_target_x(drunk))
        {
            data->first = data->second;
            data->second = NONE;
        }
    }
    else if (data->first == VERTICAL)
    {
        drunk->y += drunkard_get_dy_to_target(drunk) < 0 ? -1 : 1;

        if (drunkard_get_y(drunk) == drunkard_get_target_y(drunk))
        {
            data->first = data->second;
            data->second = NONE;
        }
    }

    return true;
}

void drunkard_tunnel_path_to_target(struct drunkard *drunk)
{
    struct tunnel_path_struct *data = (void *)drunk->path_data;

    int dx = drunkard_get_dx_to_target(drunk);
    int dy = drunkard_get_dy_to_target(drunk);

    if (dx != 0 && dy != 0)
    {
        if (drunkard_rng_chance(drunk, 0.5))
        {
            data->first = HORIZONTAL;
            data->second = VERTICAL;
        }
        else
        {
            data->first = VERTICAL;
            data->second = HORIZONTAL;
        }
    }
    else if (dx != 0)
    {
        data->first = HORIZONTAL;
        data->second = NONE;
    }
    else if (dy != 0)
    {
        data->first = VERTICAL;
        data->second = NONE;
    }

    drunk->pathing_function = tunnel_path;
}

bool drunkard_walk_path(struct drunkard *drunk)
{
    if (drunk->pathing_function)
    {
        if (!drunk->pathing_function(drunk))
        {
            drunkard_cancel_path(drunk);
        }

        return true;
    }
    else
    {
        return false;
    }
}

void drunkard_cancel_path(struct drunkard *drunk)
{
    drunk->pathing_function = NULL;
}

/******************************************************************************\
Checking functions.
\******************************************************************************/

bool drunkard_is_on_opened(struct drunkard *drunk)
{
    return drunkard_is_opened(drunk, drunk->x, drunk->y);
}

bool drunkard_is_opened_on_rect(struct drunkard *drunk, unsigned hw, unsigned hh)
{
    for (int x = ceil(drunk->x - hw) - 1; x <= floor(drunk->x + hw) + 1; ++x)
        for (int y = ceil(drunk->y - hh) - 1; y <= floor(drunk->y + hh) + 1; ++y)
            if (!IN_BOUNDS(drunk, x, y) || drunkard_is_opened(drunk, x, y))
                return true;
    return false;
}

bool drunkard_is_opened_on_circle(struct drunkard *drunk, unsigned r)
{
    for (int dx = -(int)r; dx <= (int)r; ++dx)
    {
        int h = floor(sqrt(r * r - dx * dx) - 0.00000001);
        for (int dy = -h; dy <= h; ++dy)
        {
            int x = drunkard_get_x(drunk) + dx;
            int y = drunkard_get_y(drunk) + dy;
            if (!IN_BOUNDS(drunk, x, y) || drunkard_is_opened(drunk, x, y))
                return true;
        }
    }

    return false;
}

bool drunkard_is_on_target(struct drunkard *drunk)
{
    return drunk->x == drunk->target_x && drunk->y == drunk->target_y;
}

bool drunkard_is_on_fixed(struct drunkard *drunk, int x, int y)
{
    return drunk->x == x && drunk->y == y;
}

bool drunkard_is_on_fixed_x(struct drunkard *drunk, int x)
{
    return drunk->x == x;
}

bool drunkard_is_on_fixed_y(struct drunkard *drunk, int y)
{
    return drunk->y == y;
}
