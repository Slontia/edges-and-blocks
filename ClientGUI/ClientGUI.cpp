#include "ClientGUI.h"
#include "../GameCore/GameCore.h"
#include <QSignalMapper>

std::unordered_map<int, QString> AreaButton::player2color_ =
{
  {OFFEN_PLAYER, "black"},
  {DEFEN_PLAYER, "red"},
  {NO_PLAYER, "white"}
};

const QString EdgeButton::selected_color_ = "yellow";

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
      buttons_[BLOCK_AREA][x][y] = new BlockButton(block_loc, pos, this);
      buttons_[HORI_EDGE_AREA][x][y] = new EdgeButton(block_loc, side_unit_num, pos, false, this);
      buttons_[VERT_EDGE_AREA][x][y] = new EdgeButton(block_loc, side_unit_num, pos, true, this);
    }
  }
}
