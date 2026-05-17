#pragma once

#include "core/field.hpp"
#include "core/game.hpp"
#include <cassert>
#include <vector>

namespace ttt::test_bot {

using game::DefaultFieldInitializer;
using game::MoveResult;
using game::Sign;
using game::State;

struct MoveSpec {
  Sign sign;
  int x;
  int y;
};

inline State make_empty_state(int size = 20, int win_len = 5) {
  State::Opts opts;
  opts.rows = opts.cols = size;
  opts.win_len = win_len;
  opts.max_moves = 0;
  DefaultFieldInitializer init;
  return State(opts, &init);
}

inline State apply_moves(const std::vector<MoveSpec> &moves,
                         int size = 20, int win_len = 5) {
  State state = make_empty_state(size, win_len);
  for (const auto &m : moves) {
    const MoveResult r = state.process_move(m.sign, m.x, m.y);
    assert(!game::is_dq(r));
    assert(r == MoveResult::OK || r == MoveResult::WIN);
  }
  return state;
}

// Четыре фишки sign в ряд на строке row_y, клетка (win_x, row_y) пуста; очередь хода у sign.
inline State four_in_row_open_fifth(Sign sign, int row_y, int win_x) {
  assert(win_x >= 4);
  std::vector<MoveSpec> moves;
  if (sign == Sign::X) {
    for (int x = 0; x < 4; ++x) {
      moves.push_back({Sign::X, x, row_y});
      moves.push_back({Sign::O, x, row_y + 1});
    }
  } else {
    for (int x = 0; x < 4; ++x) {
      moves.push_back({Sign::X, x, row_y + 1});
      moves.push_back({Sign::O, x, row_y});
    }
    moves.push_back({Sign::X, 4, row_y + 1});
  }
  State state = apply_moves(moves);
  assert(state.get_current_player() == sign);
  assert(state.get_value(win_x, row_y) == Sign::NONE);
  return state;
}

// У attacker четыре в ряд; ходит соперник (блокирующий).
inline State four_in_row_to_block(Sign attacker, int row_y, int open_x) {
  assert(open_x >= 4);
  std::vector<MoveSpec> moves;
  if (attacker == Sign::O) {
    for (int x = 0; x < 4; ++x) {
      moves.push_back({Sign::X, x, row_y + 1});
      moves.push_back({Sign::O, x, row_y});
    }
  } else {
    for (int x = 0; x < 4; ++x) {
      moves.push_back({Sign::O, x, row_y + 1});
      moves.push_back({Sign::X, x, row_y});
    }
  }
  State state = apply_moves(moves);
  const Sign blocker = (attacker == Sign::O) ? Sign::X : Sign::O;
  assert(state.get_current_player() == blocker);
  assert(state.get_value(open_x, row_y) == Sign::NONE);
  return state;
}

}; // namespace ttt::test_bot
