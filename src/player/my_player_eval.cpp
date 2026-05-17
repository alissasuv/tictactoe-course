#include "my_player_eval.hpp"

#include <algorithm>
#include <climits>

namespace ttt::my_player {

using game::Sign;
using game::Status;

static Sign opponent(Sign s) {
  if (s == Sign::X)
    return Sign::O;
  if (s == Sign::O)
    return Sign::X;
  return Sign::NONE;
}

static int score_window(int my_cnt, int opp_cnt, int empty_cnt, int wall_cnt) {
  if (wall_cnt > 0 || (my_cnt > 0 && opp_cnt > 0))
    return 0;
  if (my_cnt == 5)
    return 100000;
  if (opp_cnt == 5)
    return -100000;
  if (my_cnt == 4 && empty_cnt == 1)
    return 12000;
  if (opp_cnt == 4 && empty_cnt == 1)
    return -15000;
  if (my_cnt == 3 && empty_cnt == 2)
    return 800;
  if (opp_cnt == 3 && empty_cnt == 2)
    return -1200;
  if (my_cnt == 2 && empty_cnt == 3)
    return 40;
  if (opp_cnt == 2 && empty_cnt == 3)
    return -60;
  if (my_cnt == 1 && empty_cnt == 4)
    return 4;
  if (opp_cnt == 1 && empty_cnt == 4)
    return -6;
  return 0;
}

static int window_score(const State &state, int x, int y, int dx, int dy,
                        int len, Sign me) {
  int my_cnt = 0;
  int opp_cnt = 0;
  int empty_cnt = 0;
  int wall_cnt = 0;
  for (int i = 0; i < len; ++i) {
    const int cx = x + i * dx;
    const int cy = y + i * dy;
    if (cx < 0 || cy < 0 || cx >= state.get_opts().cols ||
        cy >= state.get_opts().rows)
      return 0;
    switch (state.get_value(cx, cy)) {
    case Sign::NONE:
      ++empty_cnt;
      break;
    case Sign::WALL:
      ++wall_cnt;
      break;
    case Sign::X:
    case Sign::O:
      if (state.get_value(cx, cy) == me)
        ++my_cnt;
      else
        ++opp_cnt;
      break;
    default:
      break;
    }
  }
  return score_window(my_cnt, opp_cnt, empty_cnt, wall_cnt);
}

int evaluate(const State &state, Sign me) {
  if (state.get_status() == Status::ENDED) {
    const Sign w = state.get_winner();
    if (w == me)
      return 1000000;
    if (w == Sign::NONE)
      return 0;
    return -1000000;
  }
  const int len = state.get_opts().win_len;
  const auto &opts = state.get_opts();
  int total = 0;
  static const int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

  for (int y = 0; y < opts.rows; ++y) {
    for (int x = 0; x < opts.cols; ++x) {
      const Sign v = state.get_value(x, y);
      if (v != Sign::X && v != Sign::O)
        continue;
      for (const auto &dir : dirs) {
        for (int n = 0; n < len; ++n) {
          const int sx = x - n * dir[0];
          const int sy = y - n * dir[1];
          total += window_score(state, sx, sy, dir[0], dir[1], len, me);
        }
      }
    }
  }
  return total;
}

}; // namespace ttt::my_player
