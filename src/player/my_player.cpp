#include "my_player.hpp"
#include "my_player_eval.hpp"

#include <climits>
#include <cstdlib>

namespace ttt::my_player {

using game::MoveResult;
using game::Status;

void MyPlayer::set_sign(Sign sign) { m_sign = sign; }
const char *MyPlayer::get_name() const { return m_name; }

Sign MyPlayer::opponent(Sign s) {
  if (s == Sign::X)
    return Sign::O;
  if (s == Sign::O)
    return Sign::X;
  return Sign::NONE;
}

bool MyPlayer::is_legal(const State &state, int x, int y) {
  const auto &opts = state.get_opts();
  if (x < 0 || y < 0 || x >= opts.cols || y >= opts.rows)
    return false;
  return state.get_value(x, y) == Sign::NONE;
}

static bool is_strong_attack(const State &after, Sign player, MoveResult r) {
  if (r == MoveResult::WIN && after.get_winner() == player)
    return true;
  if (r == MoveResult::OK && after.get_status() == Status::LAST_MOVE &&
      player == Sign::X)
    return true;
  return false;
}

static bool is_opponent_threat(const State &after, Sign opp, MoveResult r) {
  if (r == MoveResult::WIN && after.get_winner() == opp)
    return true;
  if (r == MoveResult::OK && after.get_status() == Status::LAST_MOVE &&
      opp == Sign::X)
    return true;
  return false;
}

std::optional<Point> MyPlayer::find_winning_move(const State &state, Sign player) {
  const auto &opts = state.get_opts();
  for (int y = 0; y < opts.rows; ++y) {
    for (int x = 0; x < opts.cols; ++x) {
      if (!is_legal(state, x, y))
        continue;
      State copy = state;
      const MoveResult r = copy.process_move(player, x, y);
      if (game::is_dq(r))
        continue;
      if (is_strong_attack(copy, player, r))
        return Point{x, y};
    }
  }
  return std::nullopt;
}

std::optional<Point> MyPlayer::find_blocking_move(const State &state, Sign me) {
  const Sign opp = opponent(me);
  const auto &opts = state.get_opts();
  for (int y = 0; y < opts.rows; ++y) {
    for (int x = 0; x < opts.cols; ++x) {
      if (!is_legal(state, x, y))
        continue;
      State copy = state;
      const MoveResult r = copy.process_move(opp, x, y);
      if (game::is_dq(r))
        continue;
      if (is_opponent_threat(copy, opp, r))
        return Point{x, y};
    }
  }
  return std::nullopt;
}

std::vector<Point> MyPlayer::collect_battle_zone(const State &state, int radius) {
  const auto &opts = state.get_opts();
  std::vector<Point> moves;
  bool has_stones = false;
  for (int y = 0; y < opts.rows; ++y) {
    for (int x = 0; x < opts.cols; ++x) {
      const Sign v = state.get_value(x, y);
      if (v == Sign::X || v == Sign::O)
        has_stones = true;
    }
  }
  if (!has_stones) {
    const int cx = opts.cols / 2;
    const int cy = opts.rows / 2;
    if (is_legal(state, cx, cy))
      moves.push_back({cx, cy});
    return moves;
  }
  for (int y = 0; y < opts.rows; ++y) {
    for (int x = 0; x < opts.cols; ++x) {
      if (!is_legal(state, x, y))
        continue;
      bool near_stone = false;
      for (int dy = -radius; dy <= radius && !near_stone; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
        if (dx == 0 && dy == 0)
          continue;
          const int nx = x + dx;
          const int ny = y + dy;
          if (nx < 0 || ny < 0 || nx >= opts.cols || ny >= opts.rows)
            continue;
          const Sign v = state.get_value(nx, ny);
          if (v == Sign::X || v == Sign::O) {
            near_stone = true;
          break;
        }
      }
    }
      if (near_stone)
        moves.push_back({x, y});
  }
  }
  return moves;
}

Point MyPlayer::greedy_eval_move(const State &state, Sign me) {
  auto candidates = collect_battle_zone(state, 2);
  if (candidates.empty())
    return fallback_move(state);

  int best_score = INT_MIN;
  Point best = candidates.front();
  for (const Point &mv : candidates) {
    State copy = state;
    const MoveResult r = copy.process_move(me, mv.x, mv.y);
    if (game::is_dq(r))
      continue;
    const int sc = evaluate(copy, me);
    if (sc > best_score) {
      best_score = sc;
      best = mv;
    }
  }
  return best;
}

Point MyPlayer::fallback_move(const State &state) {
  auto zone = collect_battle_zone(state, 2);
  if (!zone.empty())
    return zone[std::rand() % zone.size()];
  const auto &opts = state.get_opts();
  for (int y = 0; y < opts.rows; ++y) {
    for (int x = 0; x < opts.cols; ++x) {
      if (is_legal(state, x, y))
        return {x, y};
    }
  }
  return {0, 0};
}

Point MyPlayer::make_move(const State &state) {
  if (auto mv = find_winning_move(state, m_sign))
    return *mv;
  if (auto mv = find_blocking_move(state, m_sign))
    return *mv;
  return greedy_eval_move(state, m_sign);
}

}; // namespace ttt::my_player
