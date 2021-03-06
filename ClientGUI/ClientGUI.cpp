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
#include <QDir>
#include <QCloseEvent>
#include <QLabel>
#include <QDesktopServices>

ClientGUI::ClientGUI(const GameOptions& options, QWidget *parent)
    : QMainWindow(parent), game_(std::make_unique<Game>(options)), select_manager_(std::make_unique<MovingSelectManager>())
{
    ui.setupUi(this);
    setFixedSize(kWindowSize);
    setWindowTitle("Edges And Blocks");
    setWindowIcon(QIcon(QDir::currentPath() + RESOURCE_ICON));

    turning_switcher_ = new TurningSwitcher(this, kTurningLocation);
    functions_ = new GameFunctions(this, kFunctionsLocation);
    game_info_ = new GameInfo(this, kGameInfoLocation, *game_);
    board_ = new BoardWidget(options.side_len_, kBoardLocation, this);
    board_->set_hover_color(OFFEN_PLAYER);

    notification_ = new QTextEdit(this);
    notification_->move(kNotificationLocation);
    notification_->resize(160, 80);
    notification_->setEnabled(false);
    notification_->setTextColor(QColor(0, 0, 0));
    QPalette pl = notification_->palette();
    pl.setBrush(QPalette::Base, QBrush(QColor(0, 0, 0, 0)));
    notification_->setPalette(pl);

    setAutoFillBackground(true);
    QPixmap pixmap(QDir::currentPath() + RESOURCE_BACKGROUND_PIC);
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(pixmap));
    setPalette(palette);

    QMenu *game_menu = menuBar()->addMenu(tr("Game"));
    QAction *new_game_action = new QAction(tr("New Game"));
    game_menu->addAction(new_game_action);
    connect(new_game_action, SIGNAL(triggered()), this, SLOT(start_new_game()));

    QMenu* about_menu = menuBar()->addMenu(tr("Help"));
    QAction* about_action = new QAction(tr("About"));
    about_menu->addAction(about_action);
    connect(about_action, SIGNAL(triggered()), this, SLOT(show_about()));

    statusBar()->addPermanentWidget(new QLabel(options.to_string().c_str(), this));
}

void ClientGUI::show_about()
{
  if (QMessageBox::information(this, tr("About"), tr("Author: Slontia\nVersion: 1.1.0.37"), "Go to github page", "OK") == 0)
  {
    QDesktopServices::openUrl(QUrl(QLatin1String("https://github.com/Slontia/edges-and-blocks")));
  }
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
    reset_game_variety(game_->Retract());
    set_act_enable(true); // act may has been disabled when game over
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

void ClientGUI::start_new_game()
{
  QApplication::beep();
  if (QMessageBox::question(this, "New Game", "Quit and start a new game?") == QMessageBox::Yes)
  {
    parentWidget()->show();
    close();
  }
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

ClientGUINetwork::ClientGUINetwork(std::unique_ptr<ClientAsyncWrapper>&& client, const bool& is_offen, QWidget *parent)
  : client_(std::move(client)), ClientGUI(GameOptions(),parent)
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

void ClientGUINetwork::closeEvent(QCloseEvent* event)
{
  client_ = nullptr;
  event->accept();
}

void ClientGUINetwork::receive_and_process_request_async()
{
  set_act_enable(false);
  client_->receive_request_async([&](Request* const request)
  {
    if (!request)
    {
      lost_connection();
      return;
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
      switch_player();
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
      try
      {
        client_->send_request(RetractAckRequest(agree));
      }
      catch (std::exception e)
      {
        lost_connection();
        return;
      }
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
      throw game_exception(("Unknown request type " + std::to_string(request->type_)).c_str());
    }
    set_act_enable(true);
    judge_over();
    board_->set_hover_color(turning_switcher_->get_turn());
    /* If player has no edges on board, forbidden retract. */
    if (game_->get_round() <= 1) { functions_->retract_->setEnabled(false); }
    QApplication::alert(this);
  });
}

bool ClientGUINetwork::try_act(const EdgeButton* target_edge)
{
  if (!ClientGUI::try_act(target_edge)) return false;
  try
  {
    if (const EdgeButton* selected_edge = select_manager_->get_edge())
    {
      client_->send_request(MoveRequest(selected_edge->get_edge_type(), selected_edge->get_pos(),
                            target_edge->get_edge_type(), target_edge->get_pos()));
    }
    else
    {
      client_->send_request(PlaceRequest(target_edge->get_edge_type(), target_edge->get_pos()));
    }
  }
  catch (std::exception e)
  {
    lost_connection();
    return false;
  }
  receive_and_process_request_async();
  return true;
}

void ClientGUINetwork::PassButtonEvent()
{
  ClientGUI::PassButtonEvent();
  try
  {
    client_->send_request(PassRequest());
  }
  catch (std::exception e)
  {
    lost_connection();
    return;
  }
  receive_and_process_request_async();
}

void ClientGUINetwork::RetractButtonEvent()
{
  try
  {
    client_->send_request(RetractRequest());
  }
  catch (std::exception e)
  {
    lost_connection();
    return;
  }
  notification_->setText("Waiting for response...");
  receive_and_process_request_async();
}

void ClientGUINetwork::judge_over()
{
  if (game_->is_over())
  {
    QApplication::beep();
    set_act_enable(false);
    client_ = nullptr;
  }
}

void ClientGUINetwork::switch_player()
{
  turning_switcher_->switch_turn();
}

void ClientGUINetwork::lost_connection()
{
  QMessageBox::information(this, "Info", "Lost connection with your opponent.");
  set_act_enable(false);
}

ClientGUICom::ClientGUICom(const GameOptions& options, const bool is_offen, const uint32_t level, QWidget* parent)
  : ClientGUI(options, parent), ai_(*game_, is_offen ? DEFEN_PLAYER : OFFEN_PLAYER, level), p_(is_offen ? OFFEN_PLAYER : DEFEN_PLAYER)
{
  QObject::connect(&ai_, SIGNAL(act_over()), this, SLOT(com_act()));
  if (is_offen)
  {
    board_->set_hover_color(OFFEN_PLAYER);
  }
  else
  {
    set_act_enable(false);
    board_->set_hover_color(DEFEN_PLAYER);
    ai_.act_first_async();
  }
}

void ClientGUICom::com_act()
{
  for (auto& action : ai_.best_actions()) // opponent player type
  {
    impl_game_variety(action());
  }
  if (!game_->is_over())
  {
    board_->set_hover_color(p_);
    set_act_enable(true);
  }
  switch_player();
  functions_->retract_->setEnabled(game_->get_round() > 1);
}

bool ClientGUICom::try_act(const EdgeButton* target_edge)
{
  if (!ClientGUI::try_act(target_edge)) { return false; }
  if (!game_->is_over())
  {
    set_act_enable(false);
    ai_.act_async();
  } // TODO: consider retract
  return true;
}

void ClientGUICom::switch_player()
{
  turning_switcher_->switch_turn();
}

void ClientGUICom::PassButtonEvent()
{
  notification_->clear();
  game_->Pass();
  select_manager_->clear_edge();
  if (game_->get_round() > 1) { functions_->retract_->setEnabled(true); }
  switch_player();
  judge_over();
  if (!game_->is_over())
  {
    set_act_enable(false);
    ai_.act_async();
  }
}

void ClientGUICom::RetractButtonEvent()
{
  notification_->clear();
  try
  {
    if (turning_switcher_->get_turn() == p_)
    {
      // if game is not over or computer has just acted, retract twice
      reset_game_variety(game_->Retract());
    }
    else if (game_->is_over())
    {
      turning_switcher_->switch_turn();
    }
    reset_game_variety(game_->Retract());
    board_->set_hover_color(p_);
    set_act_enable(true);
    // retract two actions each time
    if (game_->get_round() <= 1) { functions_->retract_->setEnabled(false); }
  }
  catch (const game_exception& e)
  {
    notification_->setText(e.what());
  }
}
