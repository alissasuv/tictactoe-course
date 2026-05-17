#include "my_player_search.hpp"
#include "my_player_eval.hpp"

#include <algorithm>
#include <chrono>
#include <climits>

namespace ttt::my_player {

using game::MoveResult;
using game::Sign;
using game::Status;

namespace {
constexpr int kMaxCandidates = 14;
constexpr int kMaxDepth = 2;
} // namespace

static bool is_legal(const State &state, int x, int y) {
  const auto &opts = state.get_opts();
  if (x < 0 || y < 0 || x >= opts.cols || y >= opts.rows)
    return false;
  return state.get_value(x, y) == Sign::NONE;
}

std::vector<Point> generate_candidates(const State &state, int radius) {
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

static std::vector<Point> limit_candidates(const State &state, Sign me,
                                         std::vector<Point> moves) {
  if (static_cast<int>(moves.size()) <= kMaxCandidates)
    return moves;
  const Sign side = state.get_current_player();
  std::sort(moves.begin(), moves.end(), [&](const Point &a, const Point &b) {
    State sa = state;
    State sb = state;
    sa.process_move(side, a.x, a.y);
    sb.process_move(side, b.x, b.y);
    return evaluate(sa, me) > evaluate(sb, me);
  });
  moves.resize(kMaxCandidates);
  return moves;
}

static int terminal_value(const State &state, Sign me) {
  if (state.get_status() == Status::ENDED) {
    const Sign w = state.get_winner();
    if (w == me)
      return 10000000;
    if (w == Sign::NONE)
      return 0;
    return -10000000;
  }
  return evaluate(state, me);
}

static int minimax(State state, int depth, int alpha, int beta, Sign me,
                   std::chrono::steady_clock::time_point deadline, bool *timed_out) {
  if (std::chrono::steady_clock::now() >= deadline) {
    *timed_out = true;
    return evaluate(state, me);
  }
  if (depth == 0 || state.get_status() == Status::ENDED) {
    return terminal_value(state, me);
  }

  const Sign side = state.get_current_player();
  const bool maximizing = (side == me);
  auto moves = limit_candidates(state, me, generate_candidates(state, 2));
  if (moves.empty())
    return evaluate(state, me);

  if (maximizing) {
    int best = INT_MIN;
    for (const Point &mv : moves) {
      State child = state;
      const MoveResult r = child.process_move(side, mv.x, mv.y);
      if (game::is_dq(r))
        continue;
      const int val = (child.get_status() == Status::ENDED)
                          ? terminal_value(child, me)
                          : minimax(child, depth - 1, alpha, beta, me, deadline,
                                    timed_out);
      if (*timed_out)
        return best == INT_MIN ? val : best;
      best = std::max(best, val);
      alpha = std::max(alpha, best);
      if (beta <= alpha)
        break;
    }
    return best;
  }

  int best = INT_MAX;
  for (const Point &mv : moves) {
    State child = state;
    const MoveResult r = child.process_move(side, mv.x, mv.y);
    if (game::is_dq(r))
      continue;
    const int val = (child.get_status() == Status::ENDED)
                        ? terminal_value(child, me)
                        : minimax(child, depth - 1, alpha, beta, me, deadline,
                                  timed_out);
    if (*timed_out)
      return best == INT_MAX ? val : best;
    best = std::min(best, val);
    beta = std::min(beta, best);
    if (beta <= alpha)
      break;
  }
  return best;
}

SearchResult search_best_move(
    const State &state, Sign me,
    std::chrono::steady_clock::time_point deadline) {
  SearchResult result;
  auto candidates = limit_candidates(state, me, generate_candidates(state, 2));
  if (candidates.empty()) {
    const auto &opts = state.get_opts();
    const int cx = opts.cols / 2;
    const int cy = opts.rows / 2;
    if (is_legal(state, cx, cy)) {
      result.move = {cx, cy};
      return result;
    }
    for (int y = 0; y < opts.rows; ++y) {
      for (int x = 0; x < opts.cols; ++x) {
        if (is_legal(state, x, y)) {
          result.move = {x, y};
          return result;
        }
      }
    }
    return result;
  }
  result.move = candidates.front();

  for (int depth = 1; depth <= kMaxDepth; ++depth) {
    if (std::chrono::steady_clock::now() >= deadline)
      break;

  int best_score = INT_MIN;
    Point best_move = result.move;
    bool depth_completed = true;
    bool timed_out = false;

  const Sign side = state.get_current_player();
  for (const Point &mv : candidates) {
      if (std::chrono::steady_clock::now() >= deadline) {
        depth_completed = false;
        break;
      }
    State child = state;
    const MoveResult r = child.process_move(side, mv.x, mv.y);
    if (game::is_dq(r))
      continue;
      const int score =
          (child.get_status() == Status::ENDED)
                          ? terminal_value(child, me)
              : minimax(child, depth - 1, INT_MIN, INT_MAX, me, deadline,
                        &timed_out);
      if (timed_out) {
        depth_completed = false;
        break;
      }
    if (score > best_score) {
      best_score = score;
        best_move = mv;
    }
  }

    if (depth_completed) {
      result.move = best_move;
  result.score = best_score;
      result.depth_reached = depth;
  result.completed = true;
    }
  }
  return result;
}

}; // namespace ttt::my_player
