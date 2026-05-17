#include "core/state.hpp"
#include "player/my_player.hpp"
#include "test_bot_helpers.hpp"

#include <cassert>
#include <iostream>

int main() {
  using namespace ttt::test_bot;
  using namespace ttt::my_player;
  using namespace ttt::game;

  MyPlayer bot("test");
  bot.set_sign(Sign::X);

  State state = four_in_row_to_block(Sign::O, 12, 4);

  State threat = state;
  assert(!is_dq(threat.process_move(Sign::X, 0, 0)));
  assert(threat.process_move(Sign::O, 4, 12) == MoveResult::WIN);

  const Point block = bot.make_move(state);
  assert(state.get_value(block.x, block.y) == Sign::NONE);

  State trial = state;
  assert(!is_dq(trial.process_move(Sign::X, block.x, block.y)));
  assert(trial.process_move(Sign::O, 4, 12) != MoveResult::WIN);

  std::cout << "test_block_threat: OK\n";
  return 0;
}
