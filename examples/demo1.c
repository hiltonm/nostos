
#include <stdio.h>
#include <nostos/game.h>

int main (int argc, char *argv[])
{
    GAME *game = game_init ();
    game_loop (game);
    game_destroy (game);

    return EXIT_SUCCESS;
}
