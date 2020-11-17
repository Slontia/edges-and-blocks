#pragma once
#include "GameCore.h"
#include "game_recorder.h"
#include <array>

class Board;

class Area
{
public:
  const Coordinate pos_;
  const AreaType type_;
  Area(Board& board, const Coordinate& pos, const AreaType& type, std::array<int, kPlayerTypeCount>& player_counts);
  virtual ~Area();
  AreaVariety set_player(const PlayerType& p); // AreaVariety
  PlayerType get_player() const;
  bool operator==(const Area& area) const;
  bool operator!=(const Area& area) const; 

protected:
  PlayerType occu_player_;
  Board& board_;

private:
  std::array<int, kPlayerTypeCount>& player_counts_;
};

class EdgeArea;
class BlockArea;
typedef std::shared_ptr<Area> AreaPtr;
typedef std::shared_ptr<EdgeArea> EdgeAreaPtr;
typedef std::shared_ptr<BlockArea> BlockAreaPtr;

class BlockArea : public Area
{
public:
  typedef std::array<EdgeAreaPtr, kEdgeCountAdjaceBlock> AdjaceEdges;
  BlockArea(Board& board, const Coordinate& pos, std::array<int, kPlayerTypeCount>& player_counts);
  virtual ~BlockArea();
  bool is_broken() const;
  bool is_occupied_by(const PlayerType& p) const;
  EdgeAreaPtr is_captured_by(const PlayerType& p) const;
  std::array<int32_t, kPlayerTypeCount> score() const;

private:
  AdjaceEdges get_adjace_edges() const;
};

class EdgeArea : public Area
{
public:
  typedef std::array<BlockAreaPtr, kBlockCountAdjaceEdge> AdjaceBlocks;
  typedef std::array<EdgeAreaPtr, kEdgeCountAdjaceEdge> AdjaceEdges;
  EdgeArea(Board& board, const Coordinate& pos, const AreaType& edge_type, std::array<int, kPlayerTypeCount>& player_counts);
  virtual ~EdgeArea();
  AdjaceBlocks get_adjace_blocks() const;
  AdjaceEdges get_adjace_edges() const;
  BlockAreaPtr get_another_block(const BlockArea& block) const;
  bool is_adjace(const EdgeArea& edge) const;

private:
  AdjaceBlocks adjace_blocks_;
  AdjaceEdges adjace_edges_;
};
