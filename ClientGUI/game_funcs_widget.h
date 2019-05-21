#pragma once
#include <QWidget>
#include <QPushButton>

class GameFunctions : public QWidget
{
private:
  QPushButton* pass_;
  QPushButton* retreat_;
  QPushButton* draw_;
  QPushButton* surrender_;

public:
  GameFunctions(QWidget* parent, const QPoint& location);
};