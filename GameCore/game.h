#ifdef ADD_OPTION

ADD_OPTION(unsigned int, side_len, 4)
ADD_OPTION(unsigned int, winner_block_occu_count, 5)
ADD_OPTION(unsigned int, init_offen_edge_own_count, 6)
ADD_OPTION(unsigned int, init_defen_edge_own_count, 6)

#endif

#ifndef GAME_H
#define GAME_H

#include "GameCore.h"
#include "board.h"
#include "area.h"
#include <stack>
#include <functional>
#include <optional>
#include <array>
#include <string>

struct GameOptions
{
#define ADD_OPTION(type, name, default_value)\
	type name##_ = default_value;\
	template <typename type>\
	GameOptions& set_##name(type&& name) { name##_ == name; return *this; }
#include "game.h"
#undef ADD_OPTION

  std::string to_string() const
  {
    return std::string()
#define ADD_OPTION(type, name, default_value) + #name + "=" + std::to_string(name##_) + "; "
#include "game.h"
#undef ADD_OPTION
    ;
  }
};

class Game
{
public:
  static PlayerType get_oppo_player(const PlayerType& p);
  Game(const GameOptions& options);
  Game(const Game&) = default;
  ~Game();
  GameVariety Place(const AreaType& edge_type, const Coordinate& pos, const PlayerType& p);
  GameVariety Move(const AreaType& old_edge_type, const Coordinate& old_pos, const AreaType& new_edge_type, const Coordinate& new_pos, const PlayerType& p);
  GameVariety Retract();
  void Pass();
  int get_round();
  bool is_over();
  const std::array<int, kPlayerTypeCount - 1>& get_edge_own_counts() const { return edge_own_counts_; }
  const std::optional<PlayerType>& get_winner() const { return winner_; }
  const Board& get_board() const { return board_; }
  int32_t score(const PlayerType& p) const;
  const GameOptions& options() const { return options_; }

private:
  const GameOptions options_;
  Board board_;
  bool is_offen_turn_;
  std::optional<PlayerType> winner_;
  std::array<int, kPlayerTypeCount - 1> edge_own_counts_;
  std::stack<GameVariety> varieties_;
  BlockAreaPtr try_capture_block_by(GameVariety& var, BlockArea& block, const PlayerType& p);
  void capture_adjace_blocks_by(GameVariety& var, EdgeArea& edge, const PlayerType& p);
  void judge_over();
  Board::OccuCounts record_edge_occu_counts();
  void supply_edges(GameVariety& var, const Board::OccuCounts& occu_counts_record);
  GameVariety change_and_refresh(std::function<void(GameVariety&)> init_variety, EdgeArea& refresh_edge, const PlayerType& p);
};

#endif
