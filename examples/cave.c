
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
     * walkable.
     */
    drunkard_mark_1(drunk, FLOOR);
    /* Now we have to "flush" the changes. I'll talk more about what flushing
     * does later.
     */
    drunkard_flush_marks(drunk);

    /* Now we have our seed, we can start carving some cool maps. We're going
     * to carve a VERY simple cave (the drunk will only carve once).
     */
     /* Start in a random location. */
    drunkard_start_random(drunk);
     /* Here's where we use our drunk's knowledge to target a place he knows
      * he can walk. This will target our seed, since it's the only open tile.
      */
    drunkard_target_random_opened(drunk);

     /* Here's the tricky part. We're going to walk towards our target until we
      * reach it or until we come across another open tile (hardly!).
      */
    while (!drunkard_is_on_target(drunk) && !drunkard_is_on_opened(drunk))
    {
        /* Mark a plus symbol of FLOORs. The plus is a cool way to carve through
         * a map. It doesn't ever leave tight diagonals to maneuver around, and
         * it's quite spacious!
         */
        drunkard_mark_plus(drunk, FLOOR);
        /* We can step to the target in a variety of ways and with different
         * weights. The weight value goes from 0 to 1. 0 basically means the
         * drunk will be moving AWAY from his target. 1 will cause the drunk
         * to go straight towards his target. And 0.5 he'll wander aimlessly,
         * and may never reach his target. I like a value from 0.6 to 0.9. We'll
         * use 0.6 for drunk that wanders a lot.
         */
        drunkard_step_to_target(drunk, 0.6);
    }

    /* We don't need to flush our changes because we're not going to carve
     * anymore, but it's good practice to do anyway.
     *
     * Until we flush, the drunk still doesn't see what the changes we made
     * since the last flush. Which is good, if he saw them, and backtracked
     * by walking randomly, he'd think he'd found an open place. We only flush
     * when we know we reached an opened tile that the drunk didn't carve this
     * iteration.
     */
    drunkard_flush_marks(drunk);

    /* Finally, let's output our map! */
    output_map(map);

    /* Always destroy the drunk when you're done with him! */
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
