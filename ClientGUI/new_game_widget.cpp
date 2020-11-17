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
  port_edit_(new QLineEdit(this))
{
  setFixedSize(230, 135);
  setWindowTitle("New Game");
  setWindowIcon(QIcon(QDir::currentPath() + RESOURCE_ICON));

  QObject::connect(start_game_btn_, SIGNAL(clicked()), this, SLOT(start_game()));
  start_game_btn_->setFixedSize(100, 30);
  start_game_btn_->move(10, 25);
  
  game_type_gbtn_->addButton(local_game_rbtn_, LOCAL_GAME);
  QObject::connect(local_game_rbtn_, SIGNAL(clicked(bool)), this, SLOT(refresh_enable_options()));
  local_game_rbtn_->move(115, 5);
  local_game_rbtn_->setChecked(true);

  game_type_gbtn_->addButton(com_game_rbtn_, COM_GAME);
  QObject::connect(com_game_rbtn_, SIGNAL(clicked(bool)), this, SLOT(refresh_enable_options()));
  com_game_rbtn_->move(115, 25);

  game_type_gbtn_->addButton(network_game_rbtn_, NETWORK_GAME);
  QObject::connect(network_game_rbtn_, SIGNAL(clicked(bool)), this, SLOT(refresh_enable_options()));
  network_game_rbtn_->move(115, 45);

  TCHAR ip_conf_buf[20] = { 0 };
  GetPrivateProfileString(TEXT("Server"), TEXT("ip"), TEXT("127.0.0.1\0"), ip_conf_buf, 20, config_file_name);
  (new QLabel("IP Address:",this))->move(20, 70);
  QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
  QRegExp ipRegex("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$");
  ip_edit_->setValidator(new QRegExpValidator(ipRegex, ip_edit_));
  ip_edit_->setText(QString::fromWCharArray(ip_conf_buf, wcslen(ip_conf_buf)));
  ip_edit_->move(100, 75);
  ip_edit_->setFixedSize(110, 20);

  TCHAR port_conf_buf[20] = { 0 };
  GetPrivateProfileString(TEXT("Server"), TEXT("port"), TEXT("9810\0"), port_conf_buf, 20, config_file_name);
  (new QLabel("Port:", this))->move(20, 95);
  port_edit_->setValidator(new QIntValidator(port_edit_));
  port_edit_->setText(QString::fromWCharArray(port_conf_buf, wcslen(port_conf_buf)));
  port_edit_->move(100, 100);
  port_edit_->setFixedSize(40, 20);
  port_edit_->setMaxLength(5);

  refresh_enable_options();
}

void NewGameWidget::start_game()
{
  switch (game_type_gbtn_->checkedId())
  {
  case LOCAL_GAME:
    open_client_gui(std::make_shared<ClientGUI>(this));
    break;
  case COM_GAME:
    open_client_gui(std::static_pointer_cast<ClientGUI>(std::make_shared<ClientGUICom>(false, 1, this)));
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
    WritePrivateProfileString(TEXT("Server"), TEXT("ip"), ip_edit_->text().toStdWString().c_str(), TEXT(".\\config.ini"));
    WritePrivateProfileString(TEXT("Server"), TEXT("port"), port_edit_->text().toStdWString().c_str(), TEXT(".\\config.ini"));
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
  ip_edit_->setEnabled(false);
  port_edit_->setEnabled(false);
  local_game_rbtn_->setEnabled(false);
  com_game_rbtn_->setEnabled(false);
  network_game_rbtn_->setEnabled(false);
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
}

