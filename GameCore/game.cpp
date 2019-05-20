#include "stdafx.h"
#include "game.h"
#include <cassert>

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
