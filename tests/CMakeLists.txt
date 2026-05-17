cmake_minimum_required(VERSION 3.20.0)

# Always built targets (no dependency on BUILD_TTTCORE)
add_executable(test_my_player test_my_player.cpp)
target_link_libraries(test_my_player tttplayer)
add_test(NAME smoke_test_player COMMAND ./test_my_player)

add_executable(test_stats test_stats.cpp)
target_link_libraries(test_stats tttplayer)
add_test(NAME test_player_stats COMMAND ./test_stats)

# Custom bot tests (lab #7)
add_executable(test_win_in_one_move test_win_in_one_move.cpp)
target_link_libraries(test_win_in_one_move tttplayer)
add_test(NAME test_win_in_one_move COMMAND ./test_win_in_one_move)

add_executable(test_block_threat test_block_threat.cpp)
target_link_libraries(test_block_threat tttplayer)
add_test(NAME test_block_threat COMMAND ./test_block_threat)

add_executable(test_move_time_limit test_move_time_limit.cpp)
target_link_libraries(test_move_time_limit tttplayer)
add_test(NAME test_move_time_limit COMMAND ./test_move_time_limit)

# Targets that require full or prebuilt tttcore
if((BUILD_TTTCORE STREQUAL "FULL") OR (BUILD_TTTCORE STREQUAL "PREBUILT"))
  # Baseline tests
  add_executable(test_with_baseline test_baseline.cpp)
  target_link_libraries(test_with_baseline tttplayer)
  add_test(NAME test_againts_baseline COMMAND ./test_with_baseline)

  # New stat test
  add_executable(test_stats_vs_baseline test_stats_vs_baseline.cpp)
  target_link_libraries(test_stats_vs_baseline tttplayer)
  add_test(NAME test_stats_vs_baseline COMMAND ./test_stats_vs_baseline)
  
  # Human player test
  add_executable(test_my_player_vs_human test_my_player_vs_human.cpp human_player.cpp)
  target_link_libraries(test_my_player_vs_human tttplayer)
endif()
