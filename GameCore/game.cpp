#include "stdafx.h"
#include "game.h"
#include "exception.h"
#include <cassert>
#include <functional>

Game::Game() : board_(kBoardSideLen), winner_(std::nullopt), is_offen_turn_(true)
{
  edge_own_counts_[OFFEN_PLAYER] = kInitOffenEdgeOwnCount;
  edge_own_counts_[DEFEN_PLAYER] = kInitDefenEdgeOwnCount;
}

Game::~Game() {}

GameVariety Game::Place(const AreaType& edge_type, const Coordinate& pos, const PlayerType& p)
{
  game_exception::check_game_not_over(*this);
  game_exception::check_edge_type(edge_type);
  game_exception::check_pos(pos, board_);
  game_exception::check_player(p);

  if (edge_own_counts_[p] <= 0)
    throw game_exception("Player has no edges to place.");

  auto edge = board_.get_edge(pos, edge_type);
  if (edge->get_player() != NO_PLAYER)
    throw game_exception("The edge has been occupied.");

  return change_and_refresh([&](GameVariety& variety) 
  {
    variety.push(edge->set_player(p));
    --edge_own_counts_[p];
  }, *edge, p);
}

GameVariety Game::Move(const AreaType& old_edge_type, const Coordinate& old_pos, const AreaType& new_edge_type, const Coordinate& new_pos, const PlayerType& p)
{
  game_exception::check_game_not_over(*this);
  game_exception::check_edge_type(old_edge_type);
  game_exception::check_edge_type(new_edge_type);
  game_exception::check_pos(old_pos, board_);
  game_exception::check_pos(new_pos, board_);
  game_exception::check_player(p);

  EdgeAreaPtr old_edge = board_.get_edge(old_pos, old_edge_type);
  if (old_edge->get_player() != p)
    throw game_exception("Player is moving an edge which has not been occupied.");

  EdgeAreaPtr new_edge = board_.get_edge(new_pos, new_edge_type);
  if (new_edge->get_player() != NO_PLAYER)
    throw game_exception("The destination edge has been occupied.");

  if (!old_edge->is_adjace(*new_edge))
    throw game_exception("Cannot move edge there.");

  return change_and_refresh([&](GameVariety& variety) 
  {
    variety.push(old_edge->set_player(NO_PLAYER));
    variety.push(new_edge->set_player(p));
  }, *new_edge, p);
}

void Game::Pass()
{
  varieties_.push(GameVariety());
}

GameVariety Game::Retract()
{
  if (varieties_.empty())
    throw game_exception("The board is empty now.");
  GameVariety game_var = varieties_.top();
  edge_own_counts_[OFFEN_PLAYER] -= game_var.offen_own_edge_count_variety_;
  edge_own_counts_[DEFEN_PLAYER] -= game_var.defen_own_edge_count_variety_;
  board_.reset_game_variety(game_var);
  varieties_.pop();
  winner_.reset();
  return game_var;
}

int Game::get_round()
{
  return varieties_.size();
}

GameVariety Game::change_and_refresh(std::function<void(GameVariety&)> change_func, EdgeArea& refresh_edge, const PlayerType& p)
{
  GameVariety game_var;
  game_var.offen_own_edge_count_variety_ = -edge_own_counts_[OFFEN_PLAYER];
  game_var.defen_own_edge_count_variety_ = -edge_own_counts_[DEFEN_PLAYER];
  change_func(game_var);

  /* Only when our edges were captured did we supply, so we record edge_occu_counts after change_func.  */
  Board::OccuCounts occu_counts_record = record_edge_occu_counts();

  capture_adjace_blocks_by(game_var, refresh_edge, p);
  capture_adjace_blocks_by(game_var, refresh_edge, get_oppo_player(p));

  supply_edges(game_var, occu_counts_record);
  judge_over();

  game_var.offen_own_edge_count_variety_ += edge_own_counts_[OFFEN_PLAYER];
  game_var.defen_own_edge_count_variety_ += edge_own_counts_[DEFEN_PLAYER];

  varieties_.push(game_var);
  return game_var;
}

void Game::capture_adjace_blocks_by(GameVariety& variety, EdgeArea& edge, const PlayerType& p)
{
  assert(p != NO_PLAYER);
  auto blocks = edge.get_adjace_blocks();
  bool go_on_capture = false;
  do
  {
    variety.to_next_time();
    go_on_capture = false;
    for (auto& block : blocks)
    {
      if (block)
      {
        block = try_capture_block_by(variety, *block, p);
        go_on_capture = true;
      }
    }
  } while (go_on_capture);
}

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

void Game::supply_edges(GameVariety& var, const Board::OccuCounts& occu_counts_record)
{
  auto supply_edges_for = [&](const PlayerType& p)
  {
    edge_own_counts_[p] += std::max<int>(0, occu_counts_record[p] - board_.get_edge_occu_counts()[p]);
  };
  supply_edges_for(OFFEN_PLAYER);
  supply_edges_for(DEFEN_PLAYER);
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
