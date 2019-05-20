#pragma once
#include "GameCore.h"
#include "game_recorder.h"

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
  bool is_broken();
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
