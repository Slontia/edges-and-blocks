#include "stdafx.h"
#include "CppUnitTest.h"
#include "../GameCore/GameCore.h"
#include <functional>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GameCoreUnitTest
{		
	TEST_CLASS(check)
	{
	public:
    TEST_METHOD(capture_one)
    {
      Game game;
      const Board& board = game.get_board();
      
      game.Place(HORI_EDGE_AREA, Coordinate {4, 4}, OFFEN_PLAYER);
/*      Assert::IsFalse(game.get_winner().has_value());
      Assert::AreEqual(Game::kInitOffenEdgeOwnCount - 1, game.get_edge_own_counts()[OFFEN_PLAYER]);
      Assert::AreEqual(Game::kInitDefenEdgeOwnCount, game.get_edge_own_counts()[DEFEN_PLAYER]);
      
      game.Place(HORI_EDGE_AREA, Coordinate {4, 3}, OFFEN_PLAYER);
      Assert::AreEqual(Game::kInitOffenEdgeOwnCount - 2, game.get_edge_own_counts()[OFFEN_PLAYER]);
      
      game.Place(VERT_EDGE_AREA, Coordinate {4, 3}, OFFEN_PLAYER);
      Assert::AreEqual(Game::kInitOffenEdgeOwnCount - 3, game.get_edge_own_counts()[OFFEN_PLAYER]);
      Assert::AreEqual(3, board.get_edge_occu_counts()[OFFEN_PLAYER]);
      Assert::AreEqual(0, board.get_edge_occu_counts()[DEFEN_PLAYER]);
      Assert::AreEqual(0, board.get_block_occu_counts()[OFFEN_PLAYER]);
      Assert::AreEqual(0, board.get_block_occu_counts()[DEFEN_PLAYER]);
      
      game.Place(VERT_EDGE_AREA, Coordinate {5, 3}, DEFEN_PLAYER);
      Assert::AreEqual(Game::kInitOffenEdgeOwnCount - 3, game.get_edge_own_counts()[OFFEN_PLAYER]);
      Assert::AreEqual(Game::kInitDefenEdgeOwnCount, game.get_edge_own_counts()[DEFEN_PLAYER]);
      Assert::AreEqual(4, board.get_edge_occu_counts()[OFFEN_PLAYER]);
      Assert::AreEqual(0, board.get_edge_occu_counts()[DEFEN_PLAYER]);
      Assert::AreEqual(1, board.get_block_occu_counts()[OFFEN_PLAYER]);
      Assert::AreEqual(0, board.get_block_occu_counts()[DEFEN_PLAYER]);*/
    }

    TEST_METHOD(move)
    {
      Game game;
      const Board& board = game.get_board();
      game.Place(HORI_EDGE_AREA, Coordinate {4, 4}, OFFEN_PLAYER);
      game.Move(HORI_EDGE_AREA, Coordinate {4, 4}, HORI_EDGE_AREA, Coordinate {3, 4}, OFFEN_PLAYER);
      Assert::AreEqual(1, board.get_edge_occu_counts()[OFFEN_PLAYER]);
    }
  };

  TEST_CLASS(exception)
  {
    TEST_METHOD(use_up_edges)
    {
      Game game;
      const Board& board = game.get_board();
      for (unsigned int i = 0; i < Game::kInitOffenEdgeOwnCount; ++i)
      {
        game.Place(HORI_EDGE_AREA, Coordinate {0, i}, OFFEN_PLAYER);
      }
      Assert::AreEqual(0, game.get_edge_own_counts()[OFFEN_PLAYER]);
      Assert::ExpectException<game_exception>(std::bind(&Game::Place, game, HORI_EDGE_AREA, Coordinate {1, 0}, OFFEN_PLAYER));
    }
  };
}