#pragma once
#include <QWidget>
#include <QPushButton>

class GameFunctions : public QWidget
{
public:
  QPushButton* pass_;
  QPushButton* retract_;
  QPushButton* draw_;
  QPushButton* surrender_;

public:
  GameFunctions(QWidget* parent, const QPoint& location);
  void set_enable(bool enable);
};
