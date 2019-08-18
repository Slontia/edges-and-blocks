#include "ClientGUI.h"
#include "../GameCore/GameCore.h"
#include "../GameCore/exception.h"
#include <QSignalMapper>
#include "switcher.h"
#include "game_funcs_widget.h"
#include "game_info_widget.h"
#include "board_widget.h"
#include "client.h"
#include "new_game_widget.h"
#include <QMessageBox>
#include <thread>
#include <QThread>

ClientGUI::ClientGUI(QWidget *parent)
    : QMainWindow(parent), game_(std::make_unique<Game>()), select_manager_(std::make_unique<MovingSelectManager>())
{
    ui.setupUi(this);
    setFixedSize(kWindowSize);
    turning_switcher_ = new TurningSwitcher(this, kTurningLocation);
    functions_ = new GameFunctions(this, kFunctionsLocation);
    game_info_ = new GameInfo(this, kGameInfoLocation, *game_);
    board_ = new BoardWidget(this, kBoardLocation);

    notification_ = new QTextEdit(this);
    notification_->move(kNotificationLocation);
    notification_->resize(160, 80);
    notification_->setEnabled(false);

    QMenu *game_menu = menuBar()->addMenu(tr("Game"));
    QAction *new_game_action = new QAction(tr("New Game"));
    game_menu->addAction(new_game_action);
    connect(new_game_action, SIGNAL(clicked()), this, SLOT(show_new_game_widget()));
}

void ClientGUI::EdgeButtonEvent()
{
  notification_->clear();
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

bool ClientGUI::try_act(const EdgeButton* target_edge)
{
  const EdgeButton* selected_edge = select_manager_->get_edge();
  try
  {
    impl_game_variety(selected_edge ?
      game_->Move(selected_edge->get_edge_type(), selected_edge->get_pos(),
                  target_edge->get_edge_type(), target_edge->get_pos(),
                  turning_switcher_->get_turn()) :
      game_->Place(target_edge->get_edge_type(), target_edge->get_pos(),
                  turning_switcher_->get_turn()));
  }
  catch (const game_exception e)
  {
    notification_->setText(e.what());
    return false;
  }
  turning_switcher_->switch_turn();
  return true;
}

void ClientGUI::PassButtonEvent()
{
  notification_->clear();
  game_->Pass();
  select_manager_->clear_edge();
  turning_switcher_->switch_turn();
}

void ClientGUI::RetractButtonEvent()
{
  notification_->clear();
  try
  {
    GameVariety game_var = game_->Retract();
    reset_game_variety(game_var);
    turning_switcher_->switch_turn();
  }
  catch (const game_exception& e)
  {
    notification_->setText(e.what());
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

void ClientGUI::show_new_game_widget()
{
  (static_cast<NewGameWidget*>(parentWidget()))->show();
}

void ClientGUI::set_act_enable(bool enable)
{
  board_->set_enable(enable);
  functions_->set_enable(enable);
}

ClientGUINetwork::ClientGUINetwork(std::unique_ptr<ClientAsyncWrapper>& client, const bool& is_offen, QWidget *parent) : client_(std::move(client)), ClientGUI(parent)
{
  if (is_offen)
  {
    receive_and_process_request_async();
  }
}

void ClientGUINetwork::receive_and_process_request_async()
{
  set_act_enable(false);
}

void ClientGUINetwork::receive_and_process_request()
{
  client_->receive_request_async([&](const Request& request)
  {
    if (request.type_ == MOVE_REQUEST)
    {
      const MoveRequest& move_request = reinterpret_cast<const MoveRequest&>(request);

      impl_game_variety(
        game_->Move(move_request.old_edge_type_, move_request.old_pos_,
        move_request.new_edge_type_, move_request.new_pos_,
        turning_switcher_->get_turn()));
    }
    else if (request.type_ == PLACE_REQUEST)
    {
      const PlaceRequest& place_request = reinterpret_cast<const PlaceRequest&>(request);
      impl_game_variety(
        game_->Place(place_request.edge_type_, place_request.pos_, turning_switcher_->get_turn()));
    }
    else if (request.type_ == PASS_REQUEST)
    {
      game_->Pass();
    }
    else if (request.type_ == RETRACT_REQUEST)
    {
      /* TODO: handle retract */
    }
    else
    {
      /* TODO: handle unexpected requests */
    }
    turning_switcher_->switch_turn();
    set_act_enable(true);
  });
}

bool ClientGUINetwork::try_act(const EdgeButton* target_edge)
{
  if (!ClientGUI::try_act(target_edge)) return false;
  if (const EdgeButton* selected_edge = select_manager_->get_edge())
  {
    client_->send_request(MoveRequest(selected_edge->get_edge_type(), selected_edge->get_pos(),
                          target_edge->get_edge_type(), target_edge->get_pos()));
  }
  else
  {
    client_->send_request(PlaceRequest(target_edge->get_edge_type(), target_edge->get_pos()));
  }
  receive_and_process_request_async();
  return true;
}

void ClientGUINetwork::PassButtonEvent()
{
  ClientGUI::PassButtonEvent();
  client_->send_request(PassRequest());
  receive_and_process_request_async();
}
void ClientGUINetwork::RetractButtonEvent()
{
  /* TODO: send retract request */
  receive_and_process_request_async();
}
