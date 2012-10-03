
/* Around the world example. */

#include <stdio.h>

#include "drunkard.h"

#define WIDTH   80
#define HEIGHT  24

enum {WALL, FLOOR};

void output_map(unsigned map[HEIGHT][WIDTH]);

int main(void)
{
    unsigned map[HEIGHT][WIDTH] = {{WALL}};

    struct drunkard *drunk = drunkard_create((void *)map, WIDTH, HEIGHT);

    drunkard_start_random_east_edge(drunk);
    drunkard_mark_1(drunk, FLOOR);
    drunkard_flush_marks(drunk);

    drunkard_target_random_north_edge(drunk);
    while (!drunkard_is_on_target(drunk))
    {
        drunkard_mark_plus(drunk, FLOOR);
        drunkard_step_to_target(drunk, 0.8);
    }

    drunkard_target_random_west_edge(drunk);
    while (!drunkard_is_on_target(drunk))
    {
        drunkard_mark_plus(drunk, FLOOR);
        drunkard_step_to_target(drunk, 0.8);
    }

    drunkard_target_random_south_edge(drunk);
    while (!drunkard_is_on_target(drunk))
    {
        drunkard_mark_plus(drunk, FLOOR);
        drunkard_step_to_target(drunk, 0.8);
    }

    drunkard_target_random_opened(drunk);
    while (!drunkard_is_on_target(drunk) || !drunkard_is_on_opened(drunk))
    {
        drunkard_mark_plus(drunk, FLOOR);
        drunkard_step_to_target(drunk, 0.8);
    }

    output_map(map);

    drunkard_destroy(drunk);

    return 0;
}

void output_map(unsigned map[HEIGHT][WIDTH])
{
    int x;
    int y;

    for (y = 0; y < HEIGHT; ++y)
    {
        for (x = 0; x < WIDTH; ++x)
        {
            printf("%c", map[y][x] ? '.' : '#');
        }
        printf("\n");
    }
}
