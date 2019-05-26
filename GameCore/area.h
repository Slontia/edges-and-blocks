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
  bool is_broken();
  bool is_occupied_by(const PlayerType& p);
  EdgeAreaPtr is_captured_by(const PlayerType& p);

private:
  AdjaceEdges get_adjace_edges();
};

class EdgeArea : public Area
{
public:
  typedef std::array<BlockAreaPtr, kBlockCountAdjaceEdge> AdjaceBlocks;
  typedef std::array<EdgeAreaPtr, kEdgeCountAdjaceEdge> AdjaceEdges;
  EdgeArea(Board& board, const Coordinate& pos, const AreaType& edge_type, std::array<int, kPlayerTypeCount>& player_counts);
  virtual ~EdgeArea();
  AdjaceBlocks get_adjace_blocks();
  AdjaceEdges get_adjace_edges();
  BlockAreaPtr get_another_block(const BlockArea& block);
  bool is_adjace(const EdgeArea& edge);

private:
  AdjaceBlocks adjace_blocks_;
  AdjaceEdges adjace_edges_;
};
