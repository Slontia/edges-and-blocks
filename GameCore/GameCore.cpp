
#include "stdafx.h"
#include "GameCore.h"
#include <cassert>
#include <utility>
#include <optional> 
#include <functional>
#include <algorithm>
#include <iostream>

Area::Area(const Coordinate& pos, const AreaType& type, std::array<int, kPlayerTypeCount>& player_counts) : pos_(pos), type_(type), occu_player_(NO_PLAYER), player_counts_(player_counts) {}

Area::~Area() {}

AreaVariety Area::set_player(const PlayerType& p)
{
  assert(occu_player_ != p);
  AreaVariety var(*this, p);
  --player_counts_[occu_player_];
  ++player_counts_[p];  
  occu_player_ = p;
  return var;
}

PlayerType Area::get_player() const
{
  return occu_player_;
}

bool Area::operator==(const Area& area) const
{
  return pos_.x_ == area.pos_.x_ && pos_.y_ == area.pos_.y_ && type_ == area.type_;
}

bool Area::operator!=(const Area& area) const
{
  return !this->operator==(area);
}

BlockArea::BlockArea(const Coordinate& pos, std::array<int, kPlayerTypeCount>& player_counts) : Area(pos, BLOCK_AREA, player_counts){}

BlockArea::~BlockArea() {}

void BlockArea::set_adjace(AdjaceEdges&& adjace_edges)
{
  adjace_edges_ = adjace_edges;
}

bool BlockArea::is_broken()
{
  if (occu_player_ == NO_PLAYER) return false;
  for (const EdgeAreaPtr& edge : adjace_edges_)
    if (const auto player = edge->get_player(); player != occu_player_ && player!= NO_PLAYER) return true;
  return false;
}

bool BlockArea::is_occupied_by(const PlayerType& p)
{
  assert(p != NO_PLAYER); 
  for (const EdgeAreaPtr& edge : adjace_edges_)
    if (edge->get_player() != p) return false;
  return true;
}

EdgeAreaPtr BlockArea::is_captured_by(const PlayerType& p)
{
  assert(p != NO_PLAYER);
  const PlayerType oppo = (p == DEFEN_PLAYER) ? OFFEN_PLAYER : DEFEN_PLAYER; 
  EdgeAreaPtr oppo_edge = nullptr;
  for (EdgeAreaPtr edge : adjace_edges_)
  {
    if (edge->get_player() == NO_PLAYER)
      return nullptr; // has free edge
    if (edge->get_player() == oppo)
    {
      if (oppo_edge)
        return nullptr; // more than one oppo edges
      oppo_edge = edge;
    }
  }
  return oppo_edge;
}

EdgeArea::EdgeArea(const Coordinate& pos, const AreaType& edge_type, std::array<int, kPlayerTypeCount>& player_counts) : Area(pos, edge_type, player_counts)
{
  assert(edge_type == HORI_EDGE_AREA || edge_type == VERT_EDGE_AREA);
}

EdgeArea::~EdgeArea() {}

void EdgeArea::set_adjace(AdjaceBlocks&& adjace_blocks, AdjaceEdges&& adjace_edges)
{
  adjace_blocks_ = adjace_blocks;
  adjace_edges_ = adjace_edges;
}

bool EdgeArea::is_adjace(const EdgeArea& edge)
{
  for (const EdgeAreaPtr& e : adjace_edges_)
  {
    if (edge == *e)
    {
      return true;
    }
  }
  return false;
}

/* Assume block is one of the adjacent blocks.
 */
BlockAreaPtr EdgeArea::get_another_block(const BlockArea& block)
{
  BlockAreaPtr another_block = nullptr;
  for (const BlockAreaPtr& b : adjace_blocks_)
  {
    if (*b != block) // different blocks
    {
      assert(!another_block);
      another_block = b;
    }
  }
  assert(another_block);
  return another_block;
}

Board::Board(const unsigned int& side_len) : side_len_(side_len), block_occu_counts_{0}, edge_occu_counts_{0}
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

AreaVariety::AreaVariety(const Area& area, const PlayerType& new_p) : type_(area.type_), pos_(area.pos_), old_player_(area.get_player()), new_player_(new_p) {}

AreaVariety::~AreaVariety() {}

GameVariety::GameVariety()
{
  area_varieties_.emplace_back();
}

GameVariety::~GameVariety() {}

void GameVariety::push(const AreaVariety& var)
{
  area_varieties_.back().push_back(var);
}

void GameVariety::clear()
{
  area_varieties_.clear();
  area_varieties_.emplace_back();
}

void GameVariety::to_next_time()
{
  if (!area_varieties_.back().empty())
  {
    area_varieties_.emplace_back();
  }
}

Game::Game() : board_(kBoardSideLen), winner_(std::nullopt), is_offen_turn_(true)
{
  edge_own_counts_[OFFEN_PLAYER] = kInitOffenEdgeOwnCount;
  edge_own_counts_[DEFEN_PLAYER] = kInitDefenEdgeOwnCount;
}

Game::~Game() {} 

BlockAreaPtr Game::try_capture_block_by(GameVariety& variety, BlockArea& block, const PlayerType& p)
{
  assert(p != NO_PLAYER);
  if (EdgeAreaPtr captured_edge = block.is_captured_by(p))
  {
    variety.push(block.set_player(p));
    variety.push(captured_edge->set_player(p));
    return captured_edge->get_another_block(block);
  }
  if (block.is_occupied_by(p) && block.get_player() != p)
  {
    assert(block.get_player() == NO_PLAYER);
    variety.push(block.set_player(p));
  }
  else if (block.is_broken())
  {
    variety.push(block.set_player(NO_PLAYER));
  }
  return nullptr;
}

void Game::capture_adjace_blocks_by(GameVariety& variety, EdgeArea& edge, const PlayerType& p)
{
  assert(p != NO_PLAYER);
  std::array<BlockAreaPtr, kBlockCountAdjaceEdge> blocks = edge.get_adjace_blocks();
  bool go_on_capture = false;
  do
  {
    variety.to_next_time();
    go_on_capture = false;
    for (BlockAreaPtr& block : blocks)
    {
      if (block)
      {
        block = try_capture_block_by(variety, *block, p);
        go_on_capture = true;
       }
    } 
  } while (go_on_capture);
}

void Game::judge_over()
{
  assert(winner_.has_value() == false);
  int offen_count = board_.get_block_occu_counts()[OFFEN_PLAYER];
  int defen_count = board_.get_block_occu_counts()[DEFEN_PLAYER];
  if (offen_count >= kWinnerBlockOccuCount && offen_count > defen_count)
    winner_.emplace(OFFEN_PLAYER);
  else if (defen_count >= kWinnerBlockOccuCount && defen_count > offen_count)
    winner_.emplace(DEFEN_PLAYER);
  else if (board_.get_edge_occu_counts()[NO_PLAYER] == 0)
    winner_.emplace(NO_PLAYER);
}

Board::OccuCounts Game::record_edge_occu_counts()
{
  Board::OccuCounts occu_counts_record;
  auto record_count = [this, &occu_counts_record](const PlayerType& p)
  {
    occu_counts_record[p] = board_.get_edge_occu_counts()[p];
  };
  record_count(OFFEN_PLAYER);
  record_count(DEFEN_PLAYER);
  return occu_counts_record;
}

void Game::supply_edges(const Board::OccuCounts& occu_counts_record)
{
  auto supply_edges = [this, occu_counts_record](const PlayerType& p)
  {
    edge_own_counts_[p] += std::max<int>(0, occu_counts_record[p] - board_.get_edge_occu_counts()[p]);
  };
  supply_edges(OFFEN_PLAYER);
  supply_edges(DEFEN_PLAYER);
}

void check_edge_type(const AreaType& edge_type)
{
  if (edge_type != HORI_EDGE_AREA && edge_type != VERT_EDGE_AREA)
    throw game_exception("Invalid edge area type.");
}

void check_game_not_over(const Game& game)
{
  if (game.get_winner().has_value())
    throw game_exception("Game is over.");
}

void check_player(const PlayerType& player)
{
  if (player == NO_PLAYER)
    throw game_exception("Invalid player type.");
}

void check_pos(const Coordinate& pos, const Board& board)
{
  if (!board.is_valid_pos(pos))
    throw game_exception("Invalid position.");
}

PlayerType Game::get_oppo_player(const PlayerType& p)
{
  if (p == DEFEN_PLAYER)
    return OFFEN_PLAYER;
  else if (p == OFFEN_PLAYER)
    return DEFEN_PLAYER;
  assert(false);
  return NO_PLAYER;
}

GameVariety Game::Place(const AreaType& edge_type, const Coordinate& pos, const PlayerType& p)
{
  check_game_not_over(*this);
  check_edge_type(edge_type);
  check_pos(pos, board_);
  check_player(p);

  if (edge_own_counts_[p] <= 0)
    throw game_exception("Player has no edges to place.");

  EdgeAreaPtr edge = board_.get_edge(pos, edge_type);
  if (edge->get_player() != NO_PLAYER)
    throw game_exception("The edge has been occupied.");

  GameVariety variety;
  variety.push(edge->set_player(p));
  --edge_own_counts_[p];

  Board::OccuCounts occu_counts_record = record_edge_occu_counts();

  capture_adjace_blocks_by(variety, *edge, p);
  capture_adjace_blocks_by(variety, *edge, get_oppo_player(p));

  supply_edges(occu_counts_record);
  judge_over();
  return variety;
}

GameVariety Game::Move(const AreaType& old_edge_type, const Coordinate& old_pos, const AreaType& new_edge_type, const Coordinate& new_pos, const PlayerType& p)
{
  check_game_not_over(*this);
  check_edge_type(old_edge_type);
  check_edge_type(new_edge_type);
  check_pos(old_pos, board_);
  check_pos(new_pos, board_);
  check_player(p);

  EdgeAreaPtr old_edge = board_.get_edge(old_pos, old_edge_type);
  if (old_edge->get_player() != p)
    throw game_exception("Player is moving an edge which has not been occupied.");

  EdgeAreaPtr new_edge = board_.get_edge(new_pos, new_edge_type);
  if (new_edge->get_player() != NO_PLAYER)
    throw game_exception("The destination edge has been occupied.");

  if (!old_edge->is_adjace(*new_edge))
    throw game_exception("Cannot move edge there.");

  GameVariety variety;
  variety.push(old_edge->set_player(NO_PLAYER));
  variety.push(new_edge->set_player(p));

  Board::OccuCounts occu_counts_record = record_edge_occu_counts();

  capture_adjace_blocks_by(variety, *new_edge, p);
  capture_adjace_blocks_by(variety, *new_edge, get_oppo_player(p));

  supply_edges(occu_counts_record);
  judge_over();
  return variety;
}


