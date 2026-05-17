#pragma once

#include "core/game.hpp"
#include <optional>
#include <vector>

namespace ttt::my_player {

// using game::Event;
using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

class MyPlayer : public IPlayer {
  Sign m_sign = Sign::NONE;
  const char *m_name;

public:
  MyPlayer(const char *name) : m_sign(Sign::NONE), m_name(name) {}
  void set_sign(Sign sign) override;
  Point make_move(const State &state) override;
  const char *get_name() const override;

private:
  static Sign opponent(Sign s);
  static bool is_legal(const State &state, int x, int y);
  static std::optional<Point> find_winning_move(const State &state, Sign player);
  static std::optional<Point> find_blocking_move(const State &state, Sign me);
  static std::vector<Point> collect_battle_zone(const State &state, int radius);
  static Point fallback_move(const State &state);
};

}; // namespace ttt::my_player
