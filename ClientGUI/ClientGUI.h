#pragma once

#include <QtWidgets/QMainWindow>
#include <qpushbutton.h>

#include "ui_ClientGUI.h"
#include "../GameCore/GameCore.h"

class ClientGUI : public QMainWindow
{
    Q_OBJECT

public:
    ClientGUI(QWidget *parent = Q_NULLPTR);

private:
    Ui::ClientGUIClass ui;
    std::array<std::array<QPushButton, Game::kBoardSideLen>, Game::kBoardSideLen> block_buttons_;
    std::array<std::array<QPushButton, Game::kBoardSideLen>, Game::kBoardSideLen + 1> vert_edge_buttons_;
    std::array<std::array<QPushButton, Game::kBoardSideLen + 1>, Game::kBoardSideLen> hori_edge_buttons_;
    void draw_board();
};
