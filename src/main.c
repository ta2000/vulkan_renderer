#include "renderer_buffer.h"
#include "renderer_image.h"
#include "renderer_tools.h"
#include "renderer.h"
#include "game.h"

#include <stdlib.h>

int main()
{
    struct game* game = malloc(sizeof(*game));

    game_run(game);

    free(game);

    return 0;
}
