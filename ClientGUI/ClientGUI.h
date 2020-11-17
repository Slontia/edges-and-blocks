#pragma once

#include <QtWidgets/QMainWindow>
#include <QPushButton>
#include <unordered_map>
#include <QTextEdit>
#include <QThread>

#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"
#include "../GameCore/game.h"
#include "moving_selector.h"

#define RESOURCE_BACKGROUND_PIC "/resources/background.jpg"
#define RESOURCE_ICON "/resources/client.ico"

class ClientAsyncWrapper;

class ClientGUI;
class MovingSelectManager;
class GameInfo;
class TurningSwitcher;
class GameFunctions;
class EdgeButton;
class AreaButton;
class BoardWidget;
class GameOptions;

class ClientGUI : public QMainWindow
{
  Q_OBJECT

public:
  ClientGUI(const GameOptions& options, QWidget *parent = Q_NULLPTR);

public slots:
  void EdgeButtonEvent();
  virtual void PassButtonEvent();
  virtual void RetractButtonEvent();
  virtual void start_new_game();
  
protected:
  std::unique_ptr<MovingSelectManager> select_manager_;
  std::unique_ptr<Game> game_;
  TurningSwitcher* turning_switcher_;
  QTextEdit* notification_;
  GameFunctions* functions_;
  BoardWidget* board_;
  virtual bool try_act(const EdgeButton* target_edge);
  virtual void judge_over();
  void impl_game_variety(const GameVariety& game_var);
  void reset_game_variety(const GameVariety& game_var);
  void set_act_enable(bool enable);

private:
  QPushButton* pass_;
  const QSize kWindowSize = QSize(800, 600);
  const QPoint kBoardLocation = QPoint(20, 20);
  const QPoint kTurningLocation = QPoint(600, 40);
  const QPoint kGameInfoLocation = QPoint(600, 100);
  const QPoint kFunctionsLocation = QPoint(600, 270);
  const QPoint kNotificationLocation = QPoint(600, 400);
  Ui::ClientGUIClass ui;
  GameInfo* game_info_;
  virtual void switch_player();
};

class ClientGUINetwork : public ClientGUI
{
  Q_OBJECT

public:
  static constexpr unsigned int kSideLen = 4;
  ClientGUINetwork(std::unique_ptr<ClientAsyncWrapper>&& client, const bool& is_offen, QWidget* parent = Q_NULLPTR);

public slots:
  virtual void PassButtonEvent();
  virtual void RetractButtonEvent();

protected:
  virtual bool try_act(const EdgeButton* target_edge) override;
  virtual void closeEvent(QCloseEvent* event) override;

private:
  std::unique_ptr<ClientAsyncWrapper> client_;
  void receive_and_process_request_async();
  void lost_connection();
  virtual void judge_over() override;
  virtual void switch_player() override;
};

class ClientGUICom : public ClientGUI
{
  Q_OBJECT

public:
  ClientGUICom(const GameOptions& options, const bool is_offen, const uint32_t level, QWidget* parent = Q_NULLPTR);

public slots:
  virtual void RetractButtonEvent();

protected:
  virtual bool try_act(const EdgeButton* target_edge) override;

private:
  void com_act();
  virtual void switch_player() override {} // do nothing

  using Action = std::function<GameVariety()>;
  template <bool return_action>
  std::conditional_t<return_action, std::vector<Action>, int32_t> act_best_choise(const PlayerType& p, const uint32_t level)
  {
    std::vector<Action> best_action;
    int32_t best_score = INT32_MIN;

    auto try_act = [this, p, &best_action, &best_score, level](Action&& action)
    {
      action(); // place or move
      // act_best_choise return the opponent's score, our score is the negative score of the opponent's score
      const int32_t score = game_->is_over() ? INT32_MAX :
                            level > 0 ? -act_best_choise<false>(Game::get_oppo_player(p), level - 1) :
                            game_->score(p);
      if (score > best_score)
      {
        if constexpr (return_action)
        {
					best_action.clear();
					best_action.emplace_back(std::move(action));
        }
        best_score = score;
      }
      game_->Retract();
    };

    auto update_best_action = [this, p, &try_act](const EdgeArea& edge)
    {
      if (edge.get_player() == NO_PLAYER)
      {
        for (const auto& adjust_edge : edge.get_adjace_edges())
        {
          if (adjust_edge->get_player() == p)
          {
            try_act([this, &edge, adjust_edge, p] { return game_->Move(adjust_edge->type_, adjust_edge->pos_, edge.type_, edge.pos_, p); });
          }
        }
        if (game_->get_edge_own_counts()[p] > 0)
        {
          try_act([this, &edge, p] { return game_->Place(edge.type_, edge.pos_, p); });
        }
      }
    };

    for (auto x = 0; x < game_->get_board().side_len_; ++x)
    {
      for (auto y = 0; y < game_->get_board().side_len_; ++y)
      {
        const Coordinate pos(x, y);
        update_best_action(*game_->get_board().get_edge(pos, AreaType::HORI_EDGE_AREA));
        update_best_action(*game_->get_board().get_edge(pos, AreaType::VERT_EDGE_AREA));
      }
    }

    if constexpr (return_action)
    {
      return best_action;
    }
    else
    {
      return best_score;
    }
	}

  const bool is_offen_;
  const uint32_t level_;
};

