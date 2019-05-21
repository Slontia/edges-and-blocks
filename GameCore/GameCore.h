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

class game_exception : public std::exception
{
public:
  game_exception(char* msg) : std::exception(msg) {}
};

