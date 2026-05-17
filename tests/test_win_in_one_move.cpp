#include "player/my_player.hpp"
#include "test_bot_helpers.hpp"

#include <cassert>
#include <iostream>

int main() {
  using namespace ttt::test_bot;
  using namespace ttt::my_player;

  MyPlayer bot("test");
  bot.set_sign(Sign::X);

  State state = four_in_row_open_fifth(Sign::X, 10, 4);
  const Point mv = bot.make_move(state);

  assert(mv.x == 4 && mv.y == 10);
  assert(state.get_value(mv.x, mv.y) == Sign::NONE);

  State after = state;
  const auto r = after.process_move(Sign::X, mv.x, mv.y);
  assert(r == MoveResult::WIN || r == MoveResult::OK);

  std::cout << "test_win_in_one_move: OK\n";
  return 0;
}
