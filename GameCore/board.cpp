#include "stdafx.h"
#include "GameCore.h"
#include "board.h"
#include <cassert>
#include <memory>
#include "game.h"

Board::Board(const unsigned int& side_len) : side_len_(side_len), block_occu_counts_ {0}, edge_occu_counts_ {0}
{
  assert(side_len > 1);
  block_occu_counts_[NO_PLAYER] = side_len * side_len;
  edge_occu_counts_[NO_PLAYER] = side_len * side_len * 2;
  build_board();
}

Board::~Board() {}

void Board::build_board()
{
  Coordinate pos;
  for (pos.x_ = 0; pos.x_ < side_len_; ++pos.x_)
  {
    areas_[BLOCK_AREA].emplace_back();
    areas_[HORI_EDGE_AREA].emplace_back();
    areas_[VERT_EDGE_AREA].emplace_back();
    for (pos.y_ = 0; pos.y_ < side_len_; ++pos.y_)
    {
      areas_[BLOCK_AREA].back().push_back(std::make_shared<BlockArea>(*this, pos, block_occu_counts_));
      areas_[HORI_EDGE_AREA].back().push_back(std::make_shared<EdgeArea>(*this, pos, HORI_EDGE_AREA, edge_occu_counts_));
      areas_[VERT_EDGE_AREA].back().push_back(std::make_shared<EdgeArea>(*this, pos, VERT_EDGE_AREA, edge_occu_counts_));
    }
  }
}

bool Board::is_valid_pos(const Coordinate& pos) const
{
  return pos.x_ < side_len_ && pos.y_ < side_len_;
}

AreaPtr Board::get_area(const Coordinate& pos, const AreaType& type)
{
  assert(is_valid_pos(pos));
  return areas_[type][pos.x_][pos.y_];
}

AreaPtr Board::get_area_safe(const Coordinate& pos, const AreaType& type)
{
  return get_area(Coordinate(pos.x_ % side_len_, pos.y_ % side_len_), type);
}


BlockAreaPtr Board::get_block(const Coordinate& pos)
{
  return std::dynamic_pointer_cast<BlockArea>(get_area(pos, BLOCK_AREA));
}

EdgeAreaPtr Board::get_edge(const Coordinate& pos, const AreaType& edge_type)
{
  assert(edge_type == HORI_EDGE_AREA || edge_type == VERT_EDGE_AREA);
  return std::dynamic_pointer_cast<EdgeArea>(get_area(pos, edge_type));
}


const Board::OccuCounts& Board::get_block_occu_counts() const
{
  return block_occu_counts_;
}

const Board::OccuCounts& Board::get_edge_occu_counts() const
{
  return edge_occu_counts_;
}

void Board::reset_game_variety(const GameVariety& game_var)
{
  for (int i = game_var.area_varieties_.size() - 1; i >= 0; --i)
  {
    for (const auto var : game_var.area_varieties_[i])
    {
      get_area(var.pos_, var.type_)->set_player(var.old_player_);
    }
  }
}

int32_t Board::score(const PlayerType& p) const
{
  std::array<int32_t, kPlayerTypeCount> scores;
  for (const auto& blocks : areas_[BLOCK_AREA])
  {
    for (const auto& block : blocks)
    {
      const auto& block_scores = std::dynamic_pointer_cast<BlockArea>(block)->score();
      scores[0] += block_scores[0];
      scores[1] += block_scores[1];
    }
  }
  // final score = sum of block score * number of blocks opponent need to occpy
  scores[0] *= Game::kWinnerBlockOccuCount - block_occu_counts_[1];
  scores[1] *= Game::kWinnerBlockOccuCount - block_occu_counts_[0];
  return scores[0] - scores[1];
}