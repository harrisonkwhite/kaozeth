#include "game.h"

int main() {
    ZF_RunGame(InitGame, RunGameTick, RenderGame, DeinitGame);
}
