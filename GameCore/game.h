#pragma once
#include "GameCore.h"
#include "board.h"
#include <stack>
#include <functional>

class Game
{
public:
  static const unsigned int kWinnerBlockOccuCount = 5;
  static const unsigned int kInitOffenEdgeOwnCount = 6;
  static const unsigned int kInitDefenEdgeOwnCount = 6;
  static const unsigned int kBoardSideLen = 8;

private:
  Board board_;
  bool is_offen_turn_;
  std::optional<PlayerType> winner_;
  std::array<int, kPlayerTypeCount - 1> edge_own_counts_;
  std::stack<GameVariety> varieties_;

  BlockAreaPtr try_capture_block_by(GameVariety& var, BlockArea& block, const PlayerType& p);
  void capture_adjace_blocks_by(GameVariety& var, EdgeArea& edge, const PlayerType& p);
  void judge_over();
  Board::OccuCounts record_edge_occu_counts();
  void supply_edges(const Board::OccuCounts& occu_counts_record);
  GameVariety change_and_refresh(std::function<void(GameVariety&)> init_variety, EdgeArea& refresh_edge, const PlayerType& p);

public:
  static PlayerType get_oppo_player(const PlayerType& p);
  Game();
  ~Game();
  GameVariety Place(const AreaType& edge_type, const Coordinate& pos, const PlayerType& p);
  GameVariety Move(const AreaType& old_edge_type, const Coordinate& old_pos, const AreaType& new_edge_type, const Coordinate& new_pos, const PlayerType& p);
  const std::array<int, kPlayerTypeCount - 1>& get_edge_own_counts() const { return edge_own_counts_; }
  const std::optional<PlayerType>& get_winner() const { return winner_; }
  const Board& get_board() const { return board_; }
};