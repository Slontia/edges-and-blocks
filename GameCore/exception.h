#pragma once
#include <exception>
#include "GameCore.h"

class Game;
class Board;

class game_exception : public std::exception
{
public:
  static void check_edge_type(const AreaType& edge_type);
  static void check_game_not_over(const Game& game);
  static void check_player(const PlayerType& player);
  static void check_pos(const Coordinate& pos, const Board& board);
  game_exception(const char* const msg) : std::exception(msg) {}
};
