#include "game_funcs_widget.h"

GameFunctions::GameFunctions(QWidget* parent, const QPoint& location) : QWidget(parent)
{
  auto init_btn = [&](QPushButton*& btn, const QString& text, const int& index, const char* handle)
  {
    btn = new QPushButton(text, this);
    btn->resize(100, 30);
    btn->move(QPoint(0, 40 * index));
    btn->setEnabled(true);
    QObject::connect(btn, SIGNAL(clicked()), parent, handle);
  };
  init_btn(pass_, "PASS", 0, SLOT(PassButtonEvent()));
  init_btn(retreat_, "RETREAT", 1, SIGNAL(retreat_signal()));
  move(location);
  resize(100, 300);
}
