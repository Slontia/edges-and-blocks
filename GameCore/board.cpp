#include "stdafx.h"
#include "GameCore.h"
#include "board.h"
#include <cassert>

Board::Board(const unsigned int& side_len) : side_len_(side_len), block_occu_counts_ {0}, edge_occu_counts_ {0}
{
  assert(side_len > 1);
  block_occu_counts_[NO_PLAYER] = side_len * side_len;
  edge_occu_counts_[NO_PLAYER] = side_len * side_len * 2;
  build_board();
  init_board();
}

Board::~Board() {}

void Board::build_board()
{
  Coordinate pos;
  for (pos.x_ = 0; pos.x_ < side_len_; ++pos.x_)
  {
    blocks_.emplace_back();
    hori_edges_.emplace_back();
    vert_edges_.emplace_back();
    for (pos.y_ = 0; pos.y_ < side_len_; ++pos.y_)
    {
      blocks_.back().push_back(std::make_shared<BlockArea>(pos, block_occu_counts_));
      hori_edges_.back().push_back(std::make_shared<EdgeArea>(pos, HORI_EDGE_AREA, edge_occu_counts_));
      vert_edges_.back().push_back(std::make_shared<EdgeArea>(pos, VERT_EDGE_AREA, edge_occu_counts_));
    }
  }
}

void Board::init_board()
{
  for (unsigned int x = 0; x < side_len_; ++x)
  {
    for (unsigned int y = 0; y < side_len_; ++y)
    {
      blocks_[x][y]->set_adjace(
      {
        hori_edges_[x][y],
        hori_edges_[x][(y + 1) % side_len_],
        vert_edges_[x][y],
        vert_edges_[(x + 1) % side_len_][y]
      });
      hori_edges_[x][y]->set_adjace(
      {
        blocks_[x][y],
        blocks_[x][(y - 1) % side_len_]
      },
      {
        hori_edges_[(x - 1) % side_len_][y],
        hori_edges_[(x + 1) % side_len_][y],
        vert_edges_[x][y],
        vert_edges_[(x + 1) % side_len_][y],
        vert_edges_[x][(y - 1) % side_len_],
        vert_edges_[(x + 1) % side_len_][(y - 1) % side_len_]
      });
      vert_edges_[x][y]->set_adjace(
      {
        blocks_[x][y],
        blocks_[(x - 1) % side_len_][y]
      },
      {
        vert_edges_[x][(y - 1) % side_len_],
        vert_edges_[x][(y + 1) % side_len_],
        hori_edges_[x][y],
        hori_edges_[(x - 1) % side_len_][y],
        hori_edges_[x][(y + 1) % side_len_],
        hori_edges_[(x - 1) % side_len_][(y + 1) % side_len_]
      });
    }
  }
}

bool Board::is_valid_pos(const Coordinate& pos) const
{
  return pos.x_ < side_len_ && pos.y_ < side_len_;
}

BlockAreaPtr Board::get_block(const Coordinate& pos)
{
  assert(is_valid_pos(pos));
  return blocks_[pos.x_][pos.y_];
}

EdgeAreaPtr Board::get_edge(const Coordinate& pos, const AreaType& edge_type)
{
  assert(is_valid_pos(pos));
  if (edge_type == HORI_EDGE_AREA)
    return hori_edges_[pos.x_][pos.y_];
  else if (edge_type == VERT_EDGE_AREA)
    return vert_edges_[pos.x_][pos.y_];
  else
    assert(false); // not edge type
  return nullptr;
}

const Board::OccuCounts& Board::get_block_occu_counts() const
{
  return block_occu_counts_;
}

const Board::OccuCounts& Board::get_edge_occu_counts() const
{
  return edge_occu_counts_;
}