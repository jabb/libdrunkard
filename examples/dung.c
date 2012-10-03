
#include <stdio.h>

#include "drunkard.h"

/* Our map's dimensions. */
#define WIDTH   80
#define HEIGHT  24

/* By default, the drunk recognizes anything above 0 to be open space, ie
 * walkable.
 */
enum {WALL, FLOOR};

/* This function simply outputs our map to the console. */
void output_map(unsigned map[HEIGHT][WIDTH]);

int main(void)
{
    /* Create our map. Making sure the default value is our WALL. It is
     * important to note that the drunk indexes in a way that the height needs
     * to be specified first.
     */
    unsigned map[HEIGHT][WIDTH] = {{WALL}};

    /* Now we need to create our poor drunk to wonder the map. */
    struct drunkard *drunk = drunkard_create((void *)map, WIDTH, HEIGHT);

    /* A lot of the drunk's navigating is done by knowing where places he can
     * walk are. If there are no places he can walk, he'll just walk aimlessly
     * and may never connect to the rest of the map if there is one. We can't
     * allow that.
     *
     * Here we carve a "seed". This seed is a walkable tile that a drunk can
     * walk to.
     */
    /* First he'll start on a random place on the map. */
    drunkard_start_random(drunk);
    /* Then he marks the spot he's on at a "FLOOR", which the drunk knows is
     * walkable. Let's mark a room. The 3's are half widths and half heights, so
     * the room will be 7x7.
     */
    drunkard_mark_rect(drunk, 3, 3, FLOOR);
    /* Now we have to "flush" the changes. I'll talk more about what flushing
     * does later.
     */
    drunkard_flush_marks(drunk);

    /* Now we have our seed, we can start carving does cool maps. We're going
     * to carve a VERY simple dungeon. The drunk will only carve once.
     */
     /* Start in a random location. */
    drunkard_start_random(drunk);
    /* Here's where we use our drunk's knowledge to target a place he knows
      * he can walk. This will target one of our seed tiles.
      */
    drunkard_target_random_opened(drunk);

    /* Lets marks a room before we start walking. */
    drunkard_mark_rect(drunk, 2, 2, FLOOR);

    /* Here's the tricky part. We're going to generate a corridor path and
      * walk along it until we reach our target or until we encounter an open
      * tile.
      */
    drunkard_tunnel_path_to_target(drunk);
    /* dunkard_walk_path will move the drunk along the path until we're at the
     * end, then it returns false.
     */
    while (drunkard_walk_path(drunk))
    {
        /* Our check if we landed on a stopping point. */
        if (drunkard_is_on_opened(drunk))
            break;

        /* Make a single tile where we're standing. */
        drunkard_mark_1(drunk, FLOOR);


        drunkard_tunnel_path_to_target(drunk);
    }

    /* Finally, let's output our map! */
    output_map(map);

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
