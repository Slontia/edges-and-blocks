#pragma once

#include <QtWidgets/QMainWindow>
#include <qpushbutton.h>

#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"
#include <cassert>

static const int kBlockSideLength = 45;
static const int kEdgeWidth = 15;
static const int kGapWidth = 4;
static const int kUnitWidth = kBlockSideLength + kEdgeWidth + kGapWidth * 2;

static const int kZeroLoc = kEdgeWidth + kGapWidth;

typedef QPoint AreaPos;

class AreaButton : public QPushButton
{
protected:
  const AreaPos pos_;
  AreaButton(const QPoint& board_loc, const QPoint& loc_offset, const AreaPos& pos, QWidget* parent) :
    QPushButton(parent), pos_(pos)
  {
    assert(pos.x() >= 0 && pos.y() >= 0);
    move(board_loc + loc_offset + pos * kUnitWidth);
  }
};

class BlockButton : public AreaButton
{
public:
  BlockButton(const QPoint& board_loc, const AreaPos& pos, QWidget* parent) :
    AreaButton(board_loc, QPoint(kZeroLoc, kZeroLoc), pos, parent)
  {
    resize(kBlockSideLength, kBlockSideLength);
  }
};

class EdgeButton : public AreaButton
{
public:
  #define REVERSE_PARA(a, b, reverse) (reverse) ? (b) : (a), (reverse) ? (a) : (b)
  EdgeButton(const QPoint& board_loc, const int& side_unit_num, const AreaPos& pos, const bool& is_vert, QWidget* parent) :
    AreaButton(board_loc, QPoint(REVERSE_PARA(kZeroLoc, 0, is_vert)), pos, parent)
  {
    assert(pos.x() < side_unit_num && pos.y() < side_unit_num);
    const bool& is_side_edge = (is_vert ? pos.x() : pos.y()) == 0;
    if (is_side_edge)
    {
      resize(REVERSE_PARA(kBlockSideLength, kUnitWidth * side_unit_num + kEdgeWidth / 2, is_vert));
      setMask(QRegion(REVERSE_PARA(0, kEdgeWidth / 2, is_vert),
                      REVERSE_PARA(kBlockSideLength, kEdgeWidth / 2, is_vert)) +
              QRegion(REVERSE_PARA(0, kUnitWidth * side_unit_num, is_vert),
                      REVERSE_PARA(kBlockSideLength, kEdgeWidth / 2, is_vert)));
    }
    else
    {
      resize(REVERSE_PARA(kBlockSideLength, kEdgeWidth, is_vert));
    }
    
  }
  #undef REVERSE_PARA
};

class ClientGUI : public QMainWindow
{
  Q_OBJECT

public:
  ClientGUI(QWidget *parent = Q_NULLPTR);

private:
  Ui::ClientGUIClass ui;
  std::array<std::array<QPushButton*, Game::kBoardSideLen>, Game::kBoardSideLen> block_buttons_;
  std::array<std::array<QPushButton*, Game::kBoardSideLen>, Game::kBoardSideLen> vert_edge_buttons_;
  std::array<std::array<QPushButton*, Game::kBoardSideLen>, Game::kBoardSideLen> hori_edge_buttons_;
  void draw_board();
};
