/* The source file capable of creating caves like you see in the documentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <libtcod.h>
#include <libtcod_int.h>

#include "drunkard.h"

#define WIDTH   160
#define HEIGHT  100

struct tile_type
{
    int face;
    bool blocks_movement, blocks_light;
    TCOD_color_t lit, unlit;
};

enum
{
    STONE_WALL,
    DIRT_WALL,
    ALIEN_WALL,
    BLOODY_WALL,
    STONE_FLOOR,
    DIRT_FLOOR,
    ALIEN_FLOOR,
    BLOODY_FLOOR,
    WOOD_DOOR, WOOD_TABLE,
    WATER_FLOWING,
    NTILES
};

const struct tile_type tiles[NTILES] = {
    {'#', true, true, {TCOD_WHITE}, {TCOD_DARKEST_GREY}},
    {'#', true, true, {TCOD_ORANGE}, {TCOD_DARKEST_ORANGE}},
    {'%', true, true, {TCOD_PURPLE}, {TCOD_DARKEST_PURPLE}},
    {'#', true, true, {TCOD_RED}, {TCOD_DARKEST_RED}},
    {'.', false, false, {TCOD_WHITE}, {TCOD_DARKEST_GREY}},
    {'.', false, false, {TCOD_ORANGE}, {TCOD_DARKEST_ORANGE}},
    {'.', false, false, {TCOD_PURPLE}, {TCOD_DARKEST_PURPLE}},
    {'.', false, false, {TCOD_RED}, {TCOD_DARKEST_RED}},
    {'+', false, true, {TCOD_YELLOW}, {TCOD_DARKEST_YELLOW}},
    {194, false, false, {TCOD_ORANGE}, {TCOD_DARKEST_ORANGE}},
    {'~', false, false, {TCOD_BLUE}, {TCOD_DARKEST_BLUE}},
};

static unsigned map[HEIGHT][WIDTH] = {{STONE_WALL}};

void draw_tile(int x, int y, unsigned tile, bool vis)
{
    if (vis)
        TCOD_console_set_default_foreground(NULL, tiles[tile].lit);
    else
        TCOD_console_set_default_foreground(NULL, tiles[tile].unlit);
    TCOD_console_put_char(NULL, x, y, tiles[tile].face, TCOD_BKGND_NONE);
}

int carve_shrinking_square(struct drunkard *drunk, int min, int max, unsigned tile)
{
    bool marked;

    do {
        marked = drunkard_is_opened_on_rect(drunk, max, max);
        max--;
    } while (marked && max >= min);

    if (!marked)
    {
        drunkard_mark_rect(drunk, max + 1, max + 1, tile);
        return max + 1;
    }
    return 0;
}

int carve_shrinking_circle(struct drunkard *drunk, int min, int max, unsigned tile)
{
    bool marked;

    do {
        marked = drunkard_is_opened_on_circle(drunk, max);
        max--;
    } while (marked && max >= min);

    if (!marked)
    {
        drunkard_mark_circle(drunk, max + 1, tile);
        /* + 1 for the max--; in the loop */
        return max + 1;
    }
    return 0;
}

void carve_seed(struct drunkard *w)
{
    drunkard_start_fixed(w, 40, 12);
    drunkard_mark_1(w, STONE_FLOOR);
    drunkard_flush_marks(w);
}

void carve_room_and_corridor(struct drunkard *w)
{
    drunkard_start_random(w);
    drunkard_target_random_opened(w);

    int marked;

    if (drunkard_rng_chance(w, 0.5))
        marked = carve_shrinking_square(w, 3, 8, STONE_FLOOR);
    else
        marked = carve_shrinking_circle(w, 3, 8, STONE_FLOOR);

    if (marked)
    {
        int x = drunkard_get_x(w);
        int y = drunkard_get_y(w);

        drunkard_tunnel_path_to_target(w);
        while (drunkard_walk_path(w))
        {
            if (drunkard_is_on_opened(w))
                break;

            drunkard_mark_1(w, STONE_FLOOR);
        }

        drunkard_start_fixed(w, x, y);

        if (map[drunkard_get_y(w)][drunkard_get_x(w) + marked + 1] == STONE_FLOOR)
        {
            drunkard_mark(w, drunkard_get_x(w) + marked, drunkard_get_y(w), WOOD_DOOR);
        }
        if (map[drunkard_get_y(w)][drunkard_get_x(w) - marked - 1] == STONE_FLOOR)
        {
            drunkard_mark(w, drunkard_get_x(w) - marked, drunkard_get_y(w), WOOD_DOOR);
        }
        if (map[drunkard_get_y(w) + marked + 1][drunkard_get_x(w)] == STONE_FLOOR)
        {
            drunkard_mark(w, drunkard_get_x(w), drunkard_get_y(w) + marked, WOOD_DOOR);
        }
        if (map[drunkard_get_y(w) - marked - 1][drunkard_get_x(w)] == STONE_FLOOR)
        {
            drunkard_mark(w, drunkard_get_x(w), drunkard_get_y(w) - marked, WOOD_DOOR);
        }

        if (drunkard_rng_chance(w, 0.3))
        {
            for (int x = drunkard_get_x(w) - abs(marked); x <= drunkard_get_x(w) + abs(marked); ++x)
            {
                for (int y = drunkard_get_y(w) - abs(marked); y <= drunkard_get_y(w) + abs(marked); ++y)
                {
                    if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
                    {
                        if (map[y][x] == STONE_FLOOR)
                        {
                            if (drunkard_rng_chance(w, 0.5))
                                drunkard_mark(w, x, y, BLOODY_FLOOR);
                        }
                        else if (map[y][x] == STONE_WALL)
                        {
                            if (drunkard_rng_chance(w, 0.5))
                                drunkard_mark(w, x, y, BLOODY_WALL);
                        }
                    }
                }
            }
        }
        else if (drunkard_rng_chance(w, 0.2))
        {
            for (int x = drunkard_get_x(w) - abs(marked); x <= drunkard_get_x(w) + abs(marked); ++x)
            {
                for (int y = drunkard_get_y(w) - abs(marked); y <= drunkard_get_y(w) + abs(marked); ++y)
                {
                    if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
                    {
                        if (map[y][x] == STONE_FLOOR)
                        {
                            if (drunkard_rng_chance(w, 0.1))
                            {
                                drunkard_mark(w, x, y, WOOD_TABLE);
                                goto end;
                            }
                        }
                    }
                }
            }
        }
    }
end:

    drunkard_flush_marks(w);
}

void carve_randomly(struct drunkard *w)
{
    drunkard_start_random(w);
    drunkard_target_random_opened(w);

    unsigned tile;
    if (drunkard_rng_chance(w, 0.1))
        tile = ALIEN_FLOOR;
    else
        tile = DIRT_FLOOR;

    while (!drunkard_is_on_opened(w))
    {
        drunkard_mark_plus(w, tile);
        drunkard_step_to_target(w, 0.55);
    }

    drunkard_flush_marks(w);
}

void carve(struct drunkard *w)
{
    if (drunkard_rng_chance(w, 0.3))
        carve_randomly(w);
    else
        carve_room_and_corridor(w);
}

void post_carve(struct drunkard *w)
{
    int neighbors[4][2] = {
        {-1,  0},
        { 0, -1},
        { 0,  1},
        { 1,  0}
    };

    for (unsigned x = 0; x < WIDTH; ++x)
    {
        for (unsigned y = 0; y < HEIGHT; ++y)
        {
            if (map[y][x] == DIRT_FLOOR)
            {
                for (unsigned i = 0; i < 4; ++i)
                {
                    int nx = x + neighbors[i][0];
                    int ny = y + neighbors[i][1];
                    unsigned tile = map[ny][nx];
                    if (tile == STONE_WALL)
                        drunkard_mark(w, nx, ny, DIRT_WALL);
                }
            }
            else if (map[y][x] == ALIEN_FLOOR)
            {
                for (unsigned i = 0; i < 4; ++i)
                {
                    int nx = x + neighbors[i][0];
                    int ny = y + neighbors[i][1];
                    unsigned tile = map[ny][nx];
                    if (tile == STONE_WALL)
                        drunkard_mark(w, nx, ny, DIRT_WALL);
                }
            }
        }
    }

    drunkard_start_random_edge(w);
    drunkard_target_random_edge(w);

    while (!drunkard_is_on_target(w))
    {
        drunkard_mark_plus(w, WATER_FLOWING);
        drunkard_step_to_target(w, 0.8);
    }

    drunkard_flush_marks(w);
}

void generate_dungeon(struct drunkard *w)
{
    drunkard_mark_all(w, STONE_WALL);
    drunkard_flush_marks(w);

    carve_seed(w);
    int tries = HEIGHT * HEIGHT;
    while (drunkard_percent_opened(w) < 0.55 && tries --> 0)
        carve(w);

    post_carve(w);
}

void generate_fov(struct drunkard *w, TCOD_map_t fov, bool explored[HEIGHT][WIDTH])
{
    (void)w;
    for (unsigned x = 0; x < WIDTH; ++x)
    {
        for (unsigned y = 0; y < HEIGHT; ++y)
        {
            struct tile_type t = tiles[map[y][x]];
            TCOD_map_set_properties(fov, x, y, !t.blocks_light, !t.blocks_movement);
            explored[y][x] = false;
        }
    }
}

int main(void)
{
    struct drunkard *drunk = drunkard_create((void *)map, WIDTH, HEIGHT);
    drunkard_set_open_threshold(drunk, STONE_FLOOR);
    unsigned s = time(NULL);
    printf("seed: %u\n", s);
    drunkard_seed(drunk, s);

    bool explored[HEIGHT][WIDTH] = {{false}};
    TCOD_map_t fov_map = TCOD_map_new(WIDTH, HEIGHT);

    TCOD_console_init_root(WIDTH, HEIGHT, "drunkard", false, TCOD_RENDERER_SDL);
    TCOD_sys_set_fps(30);

    generate_dungeon(drunk);
    unsigned px, py;
    drunkard_random_opened(drunk, &px, &py);
    generate_fov(drunk, fov_map, explored);

    while (!TCOD_console_is_window_closed())
    {
        TCOD_key_t key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);

        switch (key.vk)
        {
        case TCODK_LEFT:
            if (TCOD_map_is_walkable(fov_map, px - 1, py))
                px--;
            printf("%d, %d\n", px, py);
            break;

        case TCODK_RIGHT:
            if (TCOD_map_is_walkable(fov_map, px + 1, py))
                px++;
            printf("%d, %d\n", px, py);
            break;

        case TCODK_UP:
            if (TCOD_map_is_walkable(fov_map, px, py - 1))
                py--;
            printf("%d, %d\n", px, py);
            break;

        case TCODK_DOWN:
            if (TCOD_map_is_walkable(fov_map, px, py + 1))
                py++;
            printf("%d, %d\n", px, py);
            break;

        case TCODK_CHAR:
            if (key.c == 'r')
            {
                generate_dungeon(drunk);
                drunkard_random_opened(drunk, &px, &py);
                generate_fov(drunk, fov_map, explored);
            }
            else if (key.c == 's')
            {
                for (unsigned x = 0; x < WIDTH; ++x)
                    for (unsigned y = 0; y < HEIGHT; ++y)
                        explored[y][x] = true;
            }
            break;

        default:
            break;
        }

        TCOD_map_compute_fov(fov_map, px, py, 20, true, FOV_BASIC);

        TCOD_console_clear(NULL);

        for (unsigned x = 0; x < WIDTH; ++x)
        {
            for (unsigned y = 0; y < HEIGHT; ++y)
            {
                unsigned tile = map[y][x];
                if (TCOD_map_is_in_fov(fov_map, x, y))
                {
                    draw_tile(x, y, tile, true);
                    explored[y][x] = true;
                }
                else if (explored[y][x])
                {
                    draw_tile(x, y, tile, false);
                }
                else
                {
                    continue;
                }
            }
        }

        TCOD_console_set_default_foreground(NULL, TCOD_red);
        TCOD_console_put_char(NULL, px, py, '@', TCOD_BKGND_NONE);

        TCOD_console_flush(false);
    }

    TCOD_map_delete(fov_map);

    drunkard_destroy(drunk);

    return EXIT_SUCCESS;
}
