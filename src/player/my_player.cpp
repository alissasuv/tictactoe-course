#include "my_player.hpp"
#include "my_player_eval.hpp"
#include "my_player_search.hpp"

#include <chrono>
#include <climits>
#include <cstdlib>

namespace ttt::my_player {

using game::MoveResult;
using game::Status;

namespace {
constexpr int kTimeLimitMs = 45;
constexpr int kCandidateRadius = 2;
} // namespace

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

static std::optional<Point> scan_tactical_moves(
    const State &state, Sign player, bool check_draw_on_last_move) {
  for (const Point &p : generate_candidates(state, 3)) {
      State copy = state;
    const MoveResult r = copy.process_move(player, p.x, p.y);
      if (game::is_dq(r))
        continue;
      if (is_strong_attack(copy, player, r))
      return p;
    if (check_draw_on_last_move && state.get_status() == Status::LAST_MOVE &&
        player == Sign::O && r == MoveResult::DRAW)
      return p;
  }
  return std::nullopt;
}

std::optional<Point> MyPlayer::find_winning_move(const State &state, Sign player) {
  return scan_tactical_moves(state, player, true);
}

std::optional<Point> MyPlayer::find_blocking_move(const State &state, Sign me) {
  const Sign opp = opponent(me);
  for (const Point &p : generate_candidates(state, 3)) {
      State copy = state;
    const MoveResult r = copy.process_move(opp, p.x, p.y);
      if (game::is_dq(r))
        continue;
      if (is_opponent_threat(copy, opp, r))
      return p;
  }
  return std::nullopt;
}

std::vector<Point> MyPlayer::collect_battle_zone(const State &state, int radius) {
  return generate_candidates(state, radius);
  }

Point MyPlayer::fallback_move(const State &state) {
  auto zone = collect_battle_zone(state, kCandidateRadius);
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

Point MyPlayer::greedy_eval_move(const State &state, Sign me) {
  auto candidates = collect_battle_zone(state, kCandidateRadius);
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

Point MyPlayer::make_move(const State &state) {
  if (state.get_status() == Status::LAST_MOVE && m_sign == Sign::O) {
    if (auto mv = find_winning_move(state, Sign::O))
      return *mv;
    if (auto mv = find_blocking_move(state, Sign::O))
      return *mv;
  }

  if (auto mv = find_winning_move(state, m_sign))
    return *mv;
  if (auto mv = find_blocking_move(state, m_sign))
    return *mv;

  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(kTimeLimitMs);
  const SearchResult search = search_best_move(state, m_sign, deadline);
  if (is_legal(state, search.move.x, search.move.y))
    return search.move;
  return fallback_move(state);
}

}; // namespace ttt::my_player
