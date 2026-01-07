#include "game.h"

int main() {
    zf::game::run(game_init, game_tick, game_render, game_deinit);
}
