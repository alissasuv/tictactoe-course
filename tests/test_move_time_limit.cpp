#include "player/my_player.hpp"
#include "test_bot_helpers.hpp"

#include <chrono>
#include <cassert>
#include <iostream>

int main() {
  using namespace ttt::test_bot;
  using namespace ttt::my_player;
  using namespace ttt::game;

  MyPlayer bot("test");
  bot.set_sign(Sign::X);

  std::vector<MoveSpec> setup;
  for (int i = 0; i < 4; ++i) {
    setup.push_back({Sign::X, i, 10});
    setup.push_back({Sign::O, i, 11});
  }
  const State state = apply_moves(setup);
  assert(state.get_current_player() == Sign::X);

  const auto t0 = std::chrono::steady_clock::now();
  const Point mv = bot.make_move(state);
  const auto t1 = std::chrono::steady_clock::now();
  const double ms =
      std::chrono::duration<double, std::milli>(t1 - t0).count();

  assert(state.get_value(mv.x, mv.y) == Sign::NONE);
  assert(ms < 100.0);

  std::cout << "test_move_time_limit: OK (" << ms << " ms)\n";
  return 0;
}
