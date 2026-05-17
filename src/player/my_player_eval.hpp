#pragma once

#include "core/game.hpp"

namespace ttt::my_player {

using game::Sign;
using game::State;

int evaluate(const State &state, Sign me);

}; // namespace ttt::my_player
