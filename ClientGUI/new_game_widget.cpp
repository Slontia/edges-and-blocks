#include "new_game_widget.h"
#include "ClientGUI.h"
#include "client.h"
#include <iostream>
#include <cassert>
#include <QDebug>
#include <QMessageBox>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QDir>
#include <QRadioButton>

static auto config_file_name = TEXT(".\\config.ini");

NewGameWidget::NewGameWidget(QWidget* parent) : QMainWindow(parent), client_(nullptr),
	start_game_btn_(new QPushButton("Start Game", this)),
	game_type_gbtn_(new QButtonGroup(this)),
  local_game_rbtn_(new QRadioButton("local_game", this)),
  com_game_rbtn_(new QRadioButton("com_game", this)),
  network_game_rbtn_(new QRadioButton("network_game", this)),
  ip_edit_(new QLineEdit(this)),
  port_edit_(new QLineEdit(this)),
  side_len_spin_(new QSpinBox(this)),
  win_blocks_spin_(new QSpinBox(this)),
  init_offen_edges_spin_(new QSpinBox(this)),
  init_defen_edges_spin_(new QSpinBox(this))
{
  setFixedSize(230, 235);
  setWindowTitle("New Game");
  setWindowIcon(QIcon(QDir::currentPath() + RESOURCE_ICON));

  QObject::connect(start_game_btn_, SIGNAL(clicked()), this, SLOT(start_game()));
  start_game_btn_->setFixedSize(100, 30);
  start_game_btn_->move(10, 25);

  auto add_game_type_option = [this](QAbstractButton* const rbtn, const QPoint& pos, const int game_type)
  {
		game_type_gbtn_->addButton(rbtn, game_type);
		QObject::connect(rbtn, SIGNAL(clicked(bool)), this, SLOT(refresh_enable_options()));
		rbtn->move(pos);
  };
  add_game_type_option(local_game_rbtn_, QPoint(115, 5), LOCAL_GAME);
  add_game_type_option(com_game_rbtn_, QPoint(115, 25), COM_GAME);
  add_game_type_option(network_game_rbtn_, QPoint(115, 45), NETWORK_GAME);
  local_game_rbtn_->setChecked(true);
  refresh_enable_options();

  auto add_network_options = [this](auto&& label_str, QLineEdit* const edit, const QValidator* const validator, const QPoint& pos, const int edit_len, auto&& default_value)
  {
		auto label = new QLabel(QString::fromWCharArray(label_str), this);
		label->move(pos);
		label->setFixedSize(70, 20);
		TCHAR buf[20] = { 0 };
		GetPrivateProfileString(TEXT("Server"), LPCTSTR(label_str), LPCTSTR(default_value), buf, 20, config_file_name);
		edit->setValidator(validator);
		edit->setText(QString::fromWCharArray(buf, wcslen(buf)));
		edit->move(pos.x() + 80, pos.y());
		edit->setFixedSize(edit_len, 20);
  };
  const QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
  const QRegExp ipRegex("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$");
  add_network_options(TEXT("IP Address"), ip_edit_, new QRegExpValidator(ipRegex, ip_edit_), QPoint(20, 75), 110, TEXT("127.0.0.1\0"));
  add_network_options(TEXT("Port"), port_edit_, new QIntValidator(port_edit_), QPoint(20, 100), 40, TEXT("9810\0"));
  port_edit_->setMaxLength(5);

  const auto add_game_options = [this](auto&& label_str, QSpinBox* spin_box, const QPoint& pos, const int min, const int max, const int default_value)
  {
		auto label = new QLabel(label_str, this);
		label->move(pos);
		label->setFixedSize(130, 20);
		spin_box->setMinimum(min);
		spin_box->setMaximum(max);
		spin_box->move(pos.x() + 150, pos.y());
		spin_box->setFixedSize(40, 20);
    spin_box->setValue(default_value);
  };
  add_game_options("Side Length", side_len_spin_, QPoint(20, 125), 2, 8, 6);
  add_game_options("Winner Block Num", win_blocks_spin_, QPoint(20, 150), 2, 10, 5);
  add_game_options("Firsthand Hold Edges", init_offen_edges_spin_, QPoint(20, 175), 4, 99, 6);
  add_game_options("Backhand Hold Edges", init_defen_edges_spin_, QPoint(20, 200), 4, 99, 6);

}

void NewGameWidget::start_game()
{
  GameOptions options;
  options.side_len_ = side_len_spin_->value();
  options.winner_block_occu_count_ = win_blocks_spin_->value();
  options.init_defen_edge_own_count_ = init_defen_edges_spin_->value();
  options.init_offen_edge_own_count_ = init_offen_edges_spin_->value();
  switch (game_type_gbtn_->checkedId())
  {
  case LOCAL_GAME:
    open_client_gui(std::make_shared<ClientGUI>(options, this));
    break;
  case COM_GAME:
    open_client_gui(std::static_pointer_cast<ClientGUI>(
      std::make_shared<ClientGUICom>(options, false, 1, this)));
    break;
  case NETWORK_GAME:
    wait_for_open_client_gui_network();
    break;
  default:
    assert(false);
  }
}

void NewGameWidget::wait_for_open_client_gui_network()
{
  network_forbidden_new_game();
  try
  {
    assert(client_ == nullptr);
    client_ = std::make_unique<ClientAsyncWrapper>(ip_edit_->text().toStdString(), port_edit_->text().toInt());
    client_->wait_for_game_start_async([&](bool is_offen)
    {
      /* We use shared_ptr instead of unique_ptr to support polymorphic. */
      auto client_gui = std::make_shared<ClientGUINetwork>(std::move(client_), is_offen, this);
      open_client_gui(std::static_pointer_cast<ClientGUI>(client_gui));
    });
    WritePrivateProfileString(TEXT("Server"), TEXT("IP Address"), ip_edit_->text().toStdWString().c_str(), config_file_name);
    WritePrivateProfileString(TEXT("Server"), TEXT("Port"), port_edit_->text().toStdWString().c_str(), config_file_name);
  }
  catch (std::exception& e)
  {
    enable_new_game();
    QMessageBox::critical(this, "Error", (QString) "Cannot connect to server. Error: " + e.what());
  }
}

void NewGameWidget::cancel_client_gui_network()
{
  client_ = nullptr;
  enable_new_game();
}

void NewGameWidget::open_client_gui(std::shared_ptr<ClientGUI>& client_gui)
{
  client_gui_ = std::move(client_gui);
  client_gui_->show();
  hide();
  enable_new_game();
}

void NewGameWidget::network_forbidden_new_game()
{
  start_game_btn_->disconnect();
  QObject::connect(start_game_btn_, SIGNAL(clicked()), this, SLOT(cancel_client_gui_network()));
  start_game_btn_->setText("Cancel");
  local_game_rbtn_->setEnabled(false);
  com_game_rbtn_->setEnabled(false);
  network_game_rbtn_->setEnabled(false);
  ip_edit_->setEnabled(false);
  port_edit_->setEnabled(false);
  side_len_spin_->setDisabled(false);
  win_blocks_spin_->setDisabled(false);
  init_offen_edges_spin_->setDisabled(false);
  init_defen_edges_spin_->setDisabled(false);
}

void NewGameWidget::enable_new_game()
{
  start_game_btn_->disconnect();
  QObject::connect(start_game_btn_, SIGNAL(clicked()), this, SLOT(start_game()));
  start_game_btn_->setEnabled(true);
  start_game_btn_->setText("Start Game");
  local_game_rbtn_->setEnabled(true);
  com_game_rbtn_->setEnabled(true);
  network_game_rbtn_->setEnabled(true);
  refresh_enable_options();
}

void NewGameWidget::refresh_enable_options()
{
  const auto current_game_type = game_type_gbtn_->checkedId();
  ip_edit_->setEnabled(current_game_type == NETWORK_GAME);
  port_edit_->setEnabled(current_game_type == NETWORK_GAME);
  side_len_spin_->setEnabled(current_game_type != NETWORK_GAME);
  win_blocks_spin_->setEnabled(current_game_type != NETWORK_GAME);
  init_offen_edges_spin_->setEnabled(current_game_type != NETWORK_GAME);
  init_defen_edges_spin_->setEnabled(current_game_type != NETWORK_GAME);
}

