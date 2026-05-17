#pragma once

#include "core/game.hpp"
#include <chrono>
#include <vector>

namespace ttt::my_player {

using game::Point;
using game::Sign;
using game::State;

struct SearchResult {
  Point move{0, 0};
  int score = 0;
  int depth_reached = 0;
  bool completed = false;
};

std::vector<Point> generate_candidates(const State &state, int radius);

SearchResult search_best_move(const State &state, Sign me,
                              std::chrono::steady_clock::time_point deadline);

}; // namespace ttt::my_player
