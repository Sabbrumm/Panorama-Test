/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QGraphicsView *graphicsView;
    QVBoxLayout *leftLayout;
    QHBoxLayout *topRow;
    QPushButton *btnLoad;
    QPushButton *btnSave;
    QPushButton *btnAddRow;
    QPushButton *btnRemoveRow;
    QHBoxLayout *headerRow;
    QLineEdit *lineMachine;
    QLineEdit *lineDate;
    QLineEdit *lineTime;
    QTableWidget *tableWidget;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1400, 900);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName("graphicsView");

        verticalLayout->addWidget(graphicsView);

        leftLayout = new QVBoxLayout();
        leftLayout->setObjectName("leftLayout");
        topRow = new QHBoxLayout();
        topRow->setObjectName("topRow");
        btnLoad = new QPushButton(centralwidget);
        btnLoad->setObjectName("btnLoad");

        topRow->addWidget(btnLoad);

        btnSave = new QPushButton(centralwidget);
        btnSave->setObjectName("btnSave");

        topRow->addWidget(btnSave);

        btnAddRow = new QPushButton(centralwidget);
        btnAddRow->setObjectName("btnAddRow");

        topRow->addWidget(btnAddRow);

        btnRemoveRow = new QPushButton(centralwidget);
        btnRemoveRow->setObjectName("btnRemoveRow");

        topRow->addWidget(btnRemoveRow);


        leftLayout->addLayout(topRow);

        headerRow = new QHBoxLayout();
        headerRow->setObjectName("headerRow");
        lineMachine = new QLineEdit(centralwidget);
        lineMachine->setObjectName("lineMachine");

        headerRow->addWidget(lineMachine);

        lineDate = new QLineEdit(centralwidget);
        lineDate->setObjectName("lineDate");

        headerRow->addWidget(lineDate);

        lineTime = new QLineEdit(centralwidget);
        lineTime->setObjectName("lineTime");

        headerRow->addWidget(lineTime);


        leftLayout->addLayout(headerRow);

        tableWidget = new QTableWidget(centralwidget);
        tableWidget->setObjectName("tableWidget");

        leftLayout->addWidget(tableWidget);


        verticalLayout->addLayout(leftLayout);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "C-NOVA Panorama", nullptr));
        btnLoad->setText(QCoreApplication::translate("MainWindow", "\320\227\320\260\320\263\321\200\321\203\320\267\320\270\321\202\321\214", nullptr));
        btnSave->setText(QCoreApplication::translate("MainWindow", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214", nullptr));
        btnAddRow->setText(QCoreApplication::translate("MainWindow", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214 \321\201\321\202\321\200\320\276\320\272\321\203", nullptr));
        btnRemoveRow->setText(QCoreApplication::translate("MainWindow", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214 \321\201\321\202\321\200\320\276\320\272\321\203", nullptr));
        lineMachine->setPlaceholderText(QCoreApplication::translate("MainWindow", "\320\235\320\276\320\274\320\265\321\200 \320\272\320\276\320\274\320\277\320\273\320\265\320\272\321\201\320\260", nullptr));
        lineDate->setPlaceholderText(QCoreApplication::translate("MainWindow", "\320\224\320\260\321\202\320\260 (\320\264\320\264.\320\234\320\234.\320\263\320\263\320\263\320\263)", nullptr));
        lineTime->setPlaceholderText(QCoreApplication::translate("MainWindow", "\320\222\321\200\320\265\320\274\321\217 (\321\207\321\207:\320\274\320\274:\321\201\321\201.\320\274\321\201)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
