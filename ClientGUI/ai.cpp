#include "ai.h"
#include <windows.h>

AI::AI(Game& game, const PlayerType p, const int level) : game_(game), p_(p), level_(level), is_over_(false) {}

AI::~AI()
{
  is_over_ = true;
  thread_.join();
}

void AI::act_first_async()
{
  if (thread_.joinable())
  {
    thread_.join();
  }
  thread_ = std::thread([this]
    {
      Sleep(1000); // sleep 3 seconds to wait the bell
      best_actions_ = std::vector<Action>
        {
          [this, side_len = game_.get_board().side_len_]
          {
            // com place an edge at center first
            return game_.Place(AreaType::HORI_EDGE_AREA, Coordinate(side_len / 2, side_len / 2), OFFEN_PLAYER);
          }
        };
      emit act_over();
    }
  );
}

void AI::act_async()
{
  if (thread_.joinable())
  {
    thread_.join();
  }
  thread_ = std::thread([this]
    {
      best_actions_.clear();
      act_best_choise<true>(p_, level_);
      emit act_over();
    }
  );
}