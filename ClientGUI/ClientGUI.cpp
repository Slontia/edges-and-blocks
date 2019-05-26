#include "ClientGUI.h"
#include "../GameCore/GameCore.h"
#include "../GameCore/exception.h"
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
    game_info_ = new GameInfo(this, kGameInfoLocation, *game_);
    board_ = new BoardWidget(this, kBoardLocation);
}

void ClientGUI::EdgeButtonEvent()
{
  auto edge = static_cast<EdgeButton*>(sender());
  if (edge->get_player() == NO_PLAYER)
  {
    /* If an empty edge was selected. */
    try_act(edge);
    select_manager_->clear_edge();
  }
  else if (edge->get_player() == turning_switcher_->get_turn() && edge != select_manager_->get_edge())
  {
    /* If an our edge was selected.  */
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
    GameVariety game_var = selected_edge ?
      game_->Move(selected_edge->get_edge_type(), selected_edge->get_pos(),
                  target_edge->get_edge_type(), target_edge->get_pos(),
                  turning_switcher_->get_turn()) :
      game_->Place(target_edge->get_edge_type(), target_edge->get_pos(),
                  turning_switcher_->get_turn());
    impl_game_variety(game_var);
    turning_switcher_->switch_turn();
  }
  catch (const game_exception exp)
  {
    // TODO: show error
  }
}

void ClientGUI::PassButtonEvent()
{
  game_->Pass();
  select_manager_->clear_edge();
  turning_switcher_->switch_turn();
}

void ClientGUI::RetractButtonEvent()
{
  try
  {
    GameVariety game_var = game_->Retract();
    reset_game_variety(game_var);
    turning_switcher_->switch_turn();
  }
  catch (const game_exception& e)
  {
    // TODO: show error
  }
}

void ClientGUI::impl_game_variety(const GameVariety& game_var)
{
  board_->impl_game_variety(game_var);
  game_info_->impl_game_variety(game_var);
  game_info_->refresh_occu_block_count();
}

void ClientGUI::reset_game_variety(const GameVariety& game_var)
{
  board_->reset_game_variety(game_var);
  game_info_->reset_game_variety(game_var);
  game_info_->refresh_occu_block_count();
}
