/********************************************************************************
** Form generated from reading UI file 'ClientGUI.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENTGUI_H
#define UI_CLIENTGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ClientGUIClass
{
public:
    QMenuBar *menuBar;
    QWidget *centralWidget;

    void setupUi(QMainWindow *ClientGUIClass)
    {
        if (ClientGUIClass->objectName().isEmpty())
            ClientGUIClass->setObjectName(QStringLiteral("ClientGUIClass"));
        ClientGUIClass->resize(600, 400);
        menuBar = new QMenuBar(ClientGUIClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        ClientGUIClass->setMenuBar(menuBar);
        centralWidget = new QWidget(ClientGUIClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        ClientGUIClass->setCentralWidget(centralWidget);

        retranslateUi(ClientGUIClass);

        QMetaObject::connectSlotsByName(ClientGUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *ClientGUIClass)
    {
        ClientGUIClass->setWindowTitle(QApplication::translate("ClientGUIClass", "ClientGUI", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ClientGUIClass: public Ui_ClientGUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENTGUI_H
