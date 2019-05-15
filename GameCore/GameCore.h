#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <array>
#include <memory>
#include <optional>

const int kPlayerTypeCount = 3;
const int kAreaTypeCount = 3;

const int kBlockCountAdjaceEdge = 2;
const int kEdgeCountAdjaceBlock = 4;
const int kEdgeCountAdjaceEdge = 6;

/* We MUST place NO_PLAYER end, since we can build both an array without or with NO_PLAYER.
 */
enum PlayerType
{
	OFFEN_PLAYER = 0, DEFEN_PLAYER = 1, NO_PLAYER
};

enum AreaType
{
	HORI_EDGE_AREA, VERT_EDGE_AREA, BLOCK_AREA
};

struct Coordinate
{
  unsigned int x_;
  unsigned int y_;
};

struct AreaVariety;

class Area
{
private:
  std::array<int, kPlayerTypeCount>& player_counts_;
protected:
  PlayerType occu_player_;
public:
  const Coordinate pos_;
  const AreaType type_;
  Area(const Coordinate& pos, const AreaType& type, std::array<int, kPlayerTypeCount>& player_counts);
  ~Area();
  AreaVariety set_player(const PlayerType& p); // AreaVariety
  PlayerType get_player() const;
  bool operator==(const Area& area) const;
  bool operator!=(const Area& area) const;
};

class EdgeArea;
typedef std::shared_ptr<EdgeArea> EdgeAreaPtr;

class BlockArea;
typedef std::shared_ptr<BlockArea> BlockAreaPtr;

class BlockArea : public Area
{
public:
  typedef std::array<EdgeAreaPtr, kEdgeCountAdjaceBlock> AdjaceEdges;
private:
  AdjaceEdges adjace_edges_;
public:
  BlockArea(const Coordinate& pos, std::array<int, kPlayerTypeCount>& player_counts);
  ~BlockArea(); 
  void set_adjace(AdjaceEdges&& adjace_edges);
  bool is_occupied_by(const PlayerType& p);
  EdgeAreaPtr is_captured_by(const PlayerType& p);
};

class EdgeArea : public Area
{
public:  
  typedef std::array<BlockAreaPtr, kBlockCountAdjaceEdge> AdjaceBlocks;
  typedef std::array<EdgeAreaPtr, kEdgeCountAdjaceEdge> AdjaceEdges;
private:
  AdjaceBlocks adjace_blocks_;
  AdjaceEdges adjace_edges_;
public:
  EdgeArea(const Coordinate& pos, const AreaType& edge_type, std::array<int, kPlayerTypeCount>& player_counts);
  ~EdgeArea();
  void set_adjace(AdjaceBlocks&& adjace_blocks, AdjaceEdges&& adjace_edges);
  BlockAreaPtr get_another_block(const BlockArea& block);
  AdjaceBlocks get_adjace_blocks()
  {
    return adjace_blocks_;
  }
  bool is_adjace(const EdgeArea& edge);
};

class Board
{
public:
  typedef std::array<int, kPlayerTypeCount> OccuCounts;
private:
  unsigned int side_len_;
  std::vector<std::vector<BlockAreaPtr>> blocks_;
  std::vector<std::vector<EdgeAreaPtr>> hori_edges_;
  std::vector<std::vector<EdgeAreaPtr>> vert_edges_;
  OccuCounts block_occu_counts_;
  OccuCounts edge_occu_counts_;
  void build_board();
  void init_board();
public:
  Board(const unsigned int& side_len);
  ~Board();
  bool is_valid_pos(const Coordinate& pos) const;
  BlockAreaPtr get_block(const Coordinate& pos);
  EdgeAreaPtr get_edge(const Coordinate& pos, const AreaType& edge_type);
  const OccuCounts& get_block_occu_counts() const;
  const OccuCounts& get_edge_occu_counts() const;
};

struct AreaVariety
{
  const AreaType type_;
  const Coordinate pos_;
  const PlayerType old_player_;
  const PlayerType new_player_;
  AreaVariety(const Area& area, const PlayerType& new_p);
  ~AreaVariety();
};

class GameVariety
{
private:
  std::vector<std::vector<AreaVariety>> area_varieties_;
public:
  GameVariety();
  ~GameVariety();
  void push(const AreaVariety& area_var);
  void clear();
  void to_next_time();
  const std::vector<std::vector<AreaVariety>> get_varieties() const { return area_varieties_; }
};

typedef std::shared_ptr<GameVariety> GameVarietyPtr;

class Game
{
public:
  static const unsigned int kWinnerBlockOccuCount = 5;
  static const unsigned int kInitOffenEdgeOwnCount = 6;
  static const unsigned int kInitDefenEdgeOwnCount = 7;
  static const unsigned int kBoardSideLen = 8;
private:
  Board board_;  
  bool is_offen_turn_;
  std::optional<PlayerType> winner_;
  std::array<int, kPlayerTypeCount - 1> edge_own_counts_;
  BlockAreaPtr try_capture_block_by(GameVariety& var, BlockArea& block, const PlayerType& p);
  void capture_adjace_blocks_by(GameVariety& var, EdgeArea& edge, const PlayerType& p);
  void judge_over();
  Board::OccuCounts record_edge_occu_counts();
  void supply_edges(const Board::OccuCounts& occu_counts_record);
public:
  static PlayerType get_oppo_player(const PlayerType& p);
  Game();
  ~Game();
  GameVariety Place(const AreaType& edge_type, const Coordinate& pos, const PlayerType& p);
  GameVariety Move(const AreaType& old_edge_type, const Coordinate& old_pos, const AreaType& new_edge_type, const Coordinate& new_pos, const PlayerType& p);
  const std::array<int, kPlayerTypeCount - 1>& get_edge_own_counts() const
  {
    return edge_own_counts_;
  }
  const std::optional<PlayerType>& get_winner() const
  {
    return winner_;
  }
  const Board& get_board() const
  {
    return board_;
  }
};

class game_exception : public std::exception
{
public:
  game_exception(char* msg) : std::exception(msg) {}
};