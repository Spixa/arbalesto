#include "game.h"

int main() {
    srand(time(nullptr));
    Game::getInstance()->run();
}