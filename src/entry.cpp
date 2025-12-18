#include "game.h"

int main() {
    zf::RunGame(GameInit, GameTick, GameRender, GameCleanup);
}
