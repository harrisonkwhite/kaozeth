#include "game.h"

int main() {
    zf::RunGame(init_game, run_game_tick, render_game, deinit_game);
}
