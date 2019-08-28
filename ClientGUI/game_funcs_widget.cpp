#include "game_funcs_widget.h"

GameFunctions::GameFunctions(QWidget* parent, const QPoint& location) : QWidget(parent)
{
  auto init_btn = [&](QPushButton*& btn, const QString& text, const int& index, const char* handle)
  {
    btn = new QPushButton(text, this);
    btn->resize(100, 30);
    btn->move(QPoint(0, 40 * index));
    QObject::connect(btn, SIGNAL(clicked()), parent, handle);
  };
  init_btn(pass_, "PASS", 0, SLOT(PassButtonEvent()));
  init_btn(retract_, "RETRACT", 1, SLOT(RetractButtonEvent()));
  retract_->setEnabled(false); /* players cannot retract at first */
  move(location);
  resize(100, 300);
}

void GameFunctions::set_enable(bool enable)
{
  pass_->setEnabled(enable);
  retract_->setEnabled(enable);
}
