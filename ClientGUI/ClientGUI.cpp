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

#define BLOCK_SIZE 5

void ClientGUI::draw_board()
{
  QSignalMapper mapper(this);
  for (int i = 0; i < Game::kBoardSideLen; ++i)
  {
    for (int j = 0; j < Game::kBoardSideLen; ++j)
    {
      QPushButton& btn = block_buttons_[i][j];
      btn.setGeometry(
        (j + 1) * BLOCK_SIZE + (j / 3) * 10 - 20,
        (i + 1) * BLOCK_SIZE + (i / 3) * 10,
        BLOCK_SIZE,
        BLOCK_SIZE
      ); // set position
      //btn.setEnabled(false);
      //btn.setFont(QFont("Times", 18, QFont::Bold)); // set fond
      //btn.setStyleSheet(CERTAIN_GRID_STYLE); // set color
      //btn.setFont(GRID_FONT);
      //QObject::connect(&btn, SIGNAL(clicked()), &mapper, SLOT(map()));
      //mapper.setMapping(btn, GET_GRIDNO(i, j));
    }
  }
  //QObject::connect(mapper, SIGNAL(mapped(int)), this, SLOT(record_button(int)));
}
