#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QVector>
#include <QPointF>
#include <cmath>
#include "csvhandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class ObjectType {
    Rectangle,
    Line,
    Point
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadFile();
    void saveFile();
    void addRow();
    void removeRow();
    void onTableSelectionChanged();
    void onSceneSelectionChanged();
    void onTableCellChanged(int row, int column);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QTableWidget  *table;
    bool isSyncingSelection = false; // защита от рекурсивных сигналов
    bool isRedrawing = false; // защита от перерисовки
    

    void drawRectangles();
    void renumberRows();
    bool hasValidationErrors(QString &msg) const;
    
    ObjectType getObjectType(int x1, int y1, int x2, int y2) const;
    
    void testPanoramaMath();
};

#endif
