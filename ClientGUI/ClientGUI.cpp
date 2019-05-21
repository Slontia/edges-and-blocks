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
    draw_board();
    turning_switcher_ = new TurningSwitcher(this, kTurningLocation);
    functions_ = new GameFunctions(this, kFunctionsLocation);
    game_info = new GameInfo(this, kGameInfoLocation, *game_);
}

void ClientGUI::draw_board()
{
  const int& side_unit_num = Game::kBoardSideLen;
  for (int x = 0; x < side_unit_num; ++x)
  {
    for (int y = 0; y < side_unit_num; ++y)
    {
      const AreaPos pos(x, y);
      buttons_[BLOCK_AREA][x][y] = new BlockButton(kBoardLocation, pos, this);
      buttons_[HORI_EDGE_AREA][x][y] = new EdgeButton(kBoardLocation, side_unit_num, pos, false, this);
      buttons_[VERT_EDGE_AREA][x][y] = new EdgeButton(kBoardLocation, side_unit_num, pos, true, this);
    }
  }
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

void ClientGUI::PassButtonEvent()
{
  select_manager_->clear_edge();
  turning_switcher_->switch_turn();
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
    handle_game_variety(var);
    turning_switcher_->switch_turn();
  }
  catch (game_exception exp)
  {
    // TODO: show error
  }
}

void ClientGUI::handle_game_variety(GameVariety& game_var)
{
  const auto& area_vars = game_var.get_varieties();
  for (const auto& vars : area_vars)
  {
    for (const AreaVariety& var : vars)
      buttons_[var.type_][var.pos_.x_][var.pos_.y_]->set_player(var.old_player_, var.new_player_);
    qApp->processEvents();
    Sleep(kSleepMs);
  }
}