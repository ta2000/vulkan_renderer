#ifndef GAME_H_
#define GAME_H_

#include <stdbool.h>

struct game
{
    bool running;
    struct renderer_resources* renderer_resources;
};

void game_run(struct game* game);
void game_update(struct game* game);
void game_render(struct game* game);

#endif
