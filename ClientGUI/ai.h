#pragma once
#include <type_traits>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include "../GameCore/game.h"
#include "ui_ClientGUI.h"

class AI : public QObject
{
  Q_OBJECT

public:
  using Action = std::function<GameVariety()>;
  AI(Game& game, const PlayerType p, const int level);
  ~AI();
  void act_first_async();
  void act_async();
  const auto& best_actions() const { return best_actions_; }

signals:
  void act_over();

private:
  template <bool record_actions>
  std::conditional_t<record_actions, void, int32_t> act_best_choise(const PlayerType& p, const uint32_t level)
  {
    if (is_over_)
    {
      if constexpr (record_actions)
      {
        return;
      }
      else
      {
				return 0;
      }
    }

    int32_t best_score = INT32_MIN;

    auto try_act = [this, p, &best_score, level](Action&& action)
    {
      action(); // place or move
      // act_best_choise return the opponent's score, our score is the negative score of the opponent's score
      const int32_t score = game_.is_over() ? INT32_MAX :
                            // TODO: is double act ? act_best_choise<true>(p, level) :
                            level > 0 ? -act_best_choise<false>(Game::get_oppo_player(p), level - 1) :
                            game_.score(p);
      if (score > best_score)
      {
        if constexpr (record_actions)
        {
          if (best_score != INT32_MIN) { best_actions_.pop_back(); }
					best_actions_.emplace_back(std::move(action));
        }
        best_score = score;
      }
      game_.Retract();
    };

    auto update_best_action = [this, p, &try_act](const EdgeArea& edge)
    {
      if (edge.get_player() == NO_PLAYER)
      {
        for (const auto& adjust_edge : edge.get_adjace_edges())
        {
          if (adjust_edge->get_player() == p)
          {
            try_act([this, &edge, adjust_edge, p] { return game_.Move(adjust_edge->type_, adjust_edge->pos_, edge.type_, edge.pos_, p); });
          }
        }
        if (game_.get_edge_own_counts()[p] > 0)
        {
          try_act([this, &edge, p] { return game_.Place(edge.type_, edge.pos_, p); });
        }
      }
    };

    for (auto x = 0; x < game_.get_board().side_len_; ++x)
    {
      for (auto y = 0; y < game_.get_board().side_len_; ++y)
      {
        const Coordinate pos(x, y);
        update_best_action(*game_.get_board().get_edge(pos, AreaType::HORI_EDGE_AREA));
        update_best_action(*game_.get_board().get_edge(pos, AreaType::VERT_EDGE_AREA));
      }
    }

    if constexpr (!record_actions)
    {
      return best_score;
    }
	}

  Game& game_;
  const PlayerType p_;
  const int level_;
  std::atomic<bool> is_over_;
  std::thread thread_;
  std::vector<Action> best_actions_;
};
