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
    board_->set_hover_color(OFFEN_PLAYER);

    notification_ = new QTextEdit(this);
    notification_->move(kNotificationLocation);
    notification_->resize(160, 80);
    notification_->setEnabled(false);

    //QMenu *game_menu = menuBar()->addMenu(tr("Game"));
    //QAction *new_game_action = new QAction(tr("New Game"));
    //game_menu->addAction(new_game_action);
    //connect(new_game_action, SIGNAL(clicked()), this, SLOT(show_new_game_widget()));
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
  functions_->retract_->setEnabled(true);
  switch_player();
  judge_over();
  return true;
}

void ClientGUI::PassButtonEvent()
{
  notification_->clear();
  game_->Pass();
  select_manager_->clear_edge();
  functions_->retract_->setEnabled(true);
  switch_player();
  judge_over();
}

void ClientGUI::RetractButtonEvent()
{
  notification_->clear();
  try
  {
    GameVariety game_var = game_->Retract();
    reset_game_variety(game_var);
    if (game_->get_round() == 0) { functions_->retract_->setEnabled(false); }
    switch_player();
  }
  catch (const game_exception& e)
  {
    notification_->setText(e.what());
  }
}

void ClientGUI::switch_player()
{
  turning_switcher_->switch_turn();
  board_->set_hover_color(turning_switcher_->get_turn());
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

void ClientGUI::judge_over()
{
  if (game_->is_over())
  {
    QApplication::beep();
    set_act_enable(false);
    /* In local game, we support retract after an over game. */
    functions_->retract_->setEnabled(true);
  }
}

ClientGUINetwork::ClientGUINetwork(std::unique_ptr<ClientAsyncWrapper>& client, const bool& is_offen, QWidget *parent) : client_(std::move(client)), ClientGUI(parent)
{
  if (is_offen)
  {
    board_->set_hover_color(OFFEN_PLAYER);
  }
  else
  {
    board_->set_hover_color(DEFEN_PLAYER);
    receive_and_process_request_async();
  }
}

void ClientGUINetwork::receive_and_process_request_async()
{
  set_act_enable(false);
  client_->receive_request_async([&](Request* const request)
  {
    if (!request)
    {
      QMessageBox::information(this, "Info", "Your opponent lost the connection, you win the game.");
      set_act_enable(false);
    }
    else if (request->type_ == MOVE_REQUEST)
    {
      const MoveRequest& move_request = *reinterpret_cast<MoveRequest* const>(request);
      impl_game_variety(
        game_->Move(move_request.old_edge_type_, move_request.old_pos_,
        move_request.new_edge_type_, move_request.new_pos_,
        turning_switcher_->get_turn()));
      switch_player();
    }
    else if (request->type_ == PLACE_REQUEST)
    {
      const PlaceRequest& place_request = *reinterpret_cast<PlaceRequest* const>(request);
      impl_game_variety(
        game_->Place(place_request.edge_type_, place_request.pos_, turning_switcher_->get_turn()));
      turning_switcher_->switch_turn();
    }
    else if (request->type_ == PASS_REQUEST)
    {
      game_->Pass();
      switch_player();
    }
    else if (request->type_ == RETRACT_REQUEST)
    {
      const bool& agree = QMessageBox::Yes == QMessageBox::question(this, "Retract", "Your opponent request for regret. Agree?");
      if (agree)
      {
        reset_game_variety(game_->Retract());
        reset_game_variety(game_->Retract());
      }
      client_->send_request(RetractAckRequest(agree));
      receive_and_process_request_async();
      return; /* Avoid switch player. */
    }
    else if (request->type_ == RETRACT_ACK_REQUEST)
    {
      if (reinterpret_cast<RetractAckRequest* const>(request)->ack_)
      {
        /* Retract twice for each player's action. */
        reset_game_variety(game_->Retract());
        reset_game_variety(game_->Retract());
        notification_->setText("Opponent agreed.");
      }
      else
      {
        notification_->setText("Opponent refused.");
      }
    }
    else
    {
      /* TODO: handle unexpected requests */
    }
    set_act_enable(true);
    judge_over();
    /* If player has no edges on board, forbidden retract. */
    if (game_->get_round() <= 1) { functions_->retract_->setEnabled(false); }
    QApplication::alert(this);
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
  client_->send_request(RetractRequest());
  notification_->setText("Waiting for response...");
  receive_and_process_request_async();
}

void ClientGUINetwork::judge_over()
{
  if (game_->is_over())
  {
    QApplication::beep();
    set_act_enable(false);
  }
}

void ClientGUINetwork::switch_player()
{
  turning_switcher_->switch_turn();
}
