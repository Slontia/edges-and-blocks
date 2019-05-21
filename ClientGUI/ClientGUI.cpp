#include "ClientGUI.h"
#include "../GameCore/GameCore.h"
#include <QSignalMapper>
#include "switcher.h"
#include "game_funcs_widget.h"
#include "game_info_widget.h"
#include "board_widget.h"


ClientGUI::ClientGUI(QWidget *parent)
    : QMainWindow(parent), game_(std::make_unique<Game>()), select_manager_(std::make_unique<MovingSelectManager>())
{
    ui.setupUi(this);
    setFixedSize(kWindowSize);
    turning_switcher_ = new TurningSwitcher(this, kTurningLocation);
    functions_ = new GameFunctions(this, kFunctionsLocation);
    game_info = new GameInfo(this, kGameInfoLocation, *game_);
    board_ = new BoardWidget(this, kBoardLocation);
}

void ClientGUI::EdgeButtonEvent()
{
  auto edge = static_cast<EdgeButton*>(sender());
  if (edge->get_player() == NO_PLAYER)
  {
    try_act(edge);
    select_manager_->clear_edge();
    game_info->refresh_info();
  }
  else if (edge->get_player() == turning_switcher_->get_turn() && edge != select_manager_->get_edge())
  {
    select_manager_->set_edge(edge);
  }
  else
  {
    select_manager_->clear_edge();
  }
}

void ClientGUI::try_act(const EdgeButton* target_edge)
{
  const EdgeButton* selected_edge = select_manager_->get_edge();
  try
  {
    GameVariety var = selected_edge ?
      game_->Move(selected_edge->get_edge_type(), selected_edge->get_pos(),
                  target_edge->get_edge_type(), target_edge->get_pos(),
                  turning_switcher_->get_turn()) :
      game_->Place(target_edge->get_edge_type(), target_edge->get_pos(),
                  turning_switcher_->get_turn());
    board_->handle_game_variety(var);
    turning_switcher_->switch_turn();
  }
  catch (game_exception exp)
  {
    // TODO: show error
  }
}

void ClientGUI::PassButtonEvent()
{
  select_manager_->clear_edge();
  turning_switcher_->switch_turn();
}

