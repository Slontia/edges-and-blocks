#include "stdafx.h"
#include "CppUnitTest.h"
#include "../GameCore/GameCore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GameCoreUnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		TEST_METHOD(TestMethod1)
		{
      Game game;
      GameVariety var = game.Place(HORI_EDGE_AREA, Coordinate {4, 4}, OFFEN_PLAYER);
      
		}
	};
}