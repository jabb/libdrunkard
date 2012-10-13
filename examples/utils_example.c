
#include <stdio.h>
#include <string.h>

#include "drunkard.h"
#include "drunkard_utils.h"


#define WIDTH   80
#define HEIGHT  24

enum {WALL, FLOOR};

void output_map(unsigned map[HEIGHT][WIDTH]);

int main(void)
{
    unsigned map[HEIGHT][WIDTH] = {{WALL}};

    struct drunkard *drunk = drunkard_create((void *)map, WIDTH, HEIGHT);

    struct drunkard_plans plans = drunkard_make_plans();
    plans.min_percent_open = 0.4;
    //plans.max_iterations = 1;
    plans.first_open_tile = 1;
    plans.default_wall_tile = WALL;
    plans.default_floor_tile = FLOOR;
    plans.border = true;

    drunkard_plans_add_cave(&plans, 1, FLOOR, 0.5);
    drunkard_plans_add_room_and_corridor(&plans, 5, FLOOR, 3, 5);

    drunkard_carve_plans(drunk, &plans);

    output_map(map);

    drunkard_unmake_plans(&plans);
    drunkard_destroy(drunk);

    return 0;
}

void output_map(unsigned map[HEIGHT][WIDTH])
{
    int x, y;
    for (y = 0; y < HEIGHT; ++y)
    {
        for (x = 0; x < WIDTH; ++x)
        {
            printf("%c", map[y][x] ? '.' : '#');
        }
        printf("\n");
    }
}

