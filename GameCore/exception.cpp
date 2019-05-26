#include "stdafx.h"
#include "exception.h"
#include "game.h"

void game_exception::check_edge_type(const AreaType& edge_type)
{
  if (edge_type != HORI_EDGE_AREA && edge_type != VERT_EDGE_AREA)
    throw game_exception("Invalid edge area type.");
}

void game_exception::check_game_not_over(const Game& game)
{
  if (game.get_winner().has_value())
    throw game_exception("Game is over.");
}

void game_exception::check_player(const PlayerType& player)
{
  if (player == NO_PLAYER)
    throw game_exception("Invalid player type.");
}

void game_exception::check_pos(const Coordinate& pos, const Board& board)
{
  if (!board.is_valid_pos(pos))
    throw game_exception("Invalid position.");
}
