#include "ClientGUI.h"
#include "../GameCore/GameCore.h"
#include <QSignalMapper>

ClientGUI::ClientGUI(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFixedSize(1000, 1000);
    draw_board();
}



void ClientGUI::draw_board()
{
  const QPoint block_loc(100, 100);
  const int& side_unit_num = Game::kBoardSideLen;
  for (int x = 0; x < side_unit_num; ++x)
  {
    for (int y = 0; y < side_unit_num; ++y)
    {
      const AreaPos pos(x, y);
      block_buttons_[x][y] = new BlockButton(block_loc, pos, this);
      hori_edge_buttons_[x][y] = new EdgeButton(block_loc, side_unit_num, pos, false, this);
      vert_edge_buttons_[x][y] = new EdgeButton(block_loc, side_unit_num, pos, true, this);
    }
  }
}
