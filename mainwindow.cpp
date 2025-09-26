#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "csvhandler.h"
#include <QFileDialog>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QAbstractGraphicsShapeItem>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QBrush>
#include <QDateTime>

static const int ROW_ROLE = 1;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // таблица
    table = ui->tableWidget;
    table->setColumnCount(7);
    QStringList headers = {"№","X л.в.","Y л.в.","X н.п.","Y н.п.","ΔАзимут","ΔУгол"};
    table->setHorizontalHeaderLabels(headers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::MultiSelection);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    table->setMinimumWidth(650);

    // панорама
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setSceneRect(0,0,3840,512);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    scene->setBackgroundBrush(QBrush(Qt::black));
    
    // тесты
    // testPanoramaMath();
    
    // кнопки
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::loadFile);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveFile);
    connect(ui->btnAddRow, &QPushButton::clicked, this, &MainWindow::addRow);
    connect(ui->btnRemoveRow, &QPushButton::clicked, this, &MainWindow::removeRow);

    connect(table->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onTableSelectionChanged);

    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSceneSelectionChanged);
    connect(table, &QTableWidget::cellChanged, this, &MainWindow::onTableCellChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::loadFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Загрузить CSV", "", "CSV Files (*.csv);;All Files (*.*)");
    if (fileName.isEmpty()) return;

    CsvHandler handler;
    CsvHandler::Result res;
    QString error;
    if (!handler.load(fileName, res, error)) {
        QMessageBox::critical(this, "Ошибка загрузки", error);
        return;
    }
    
    if (!CsvHandler::autoFixResult(res, error)) {
        QMessageBox::warning(this, "Auto-fix", "Ошибка при автоисправлении: " + error);
        return;
    }

    table->blockSignals(true);
    table->clearContents();
    table->setRowCount(0);
    for (int i=0; i<res.records.size(); ++i) {
        const auto &r = res.records[i];
        int row = table->rowCount();
        table->insertRow(row);
        {
            auto *it = new QTableWidgetItem(QString::number(row+1));
            it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            table->setItem(row, 0, it);
        }
        table->setItem(row, 1, new QTableWidgetItem(QString::number(r.x1)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(r.y1)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(r.x2)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(r.y2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(r.azimuth, 'f', 2)));
        table->setItem(row, 6, new QTableWidgetItem(QString::number(r.elevation, 'f', 2)));
    }
    table->blockSignals(false);

    // поля header
    ui->lineMachine->setText(QString::number(res.header.machineNumber));
    if (ui->lineDate) ui->lineDate->setText(res.header.date.toString("dd.MM.yyyy"));
    if (ui->lineTime) ui->lineTime->setText(res.header.time.toString("HH:mm:ss.zzz"));

    drawRectangles();
    QMessageBox::information(this, "Загрузка", "Файл загружен: " + fileName);
}

void MainWindow::saveFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить CSV", "", "CSV Files (*.csv);;All Files (*.*)");
    if (fileName.isEmpty()) return;

    QString validationMsg;
    if (hasValidationErrors(validationMsg)) {
        QMessageBox::critical(this, "Невозможно сохранить", validationMsg);
        return;
    }

    CsvHandler::Result res;
    // заголовок
    res.header.machineNumber = ui->lineMachine->text().trimmed().toInt();
    if (ui->lineDate) res.header.date = QDate::fromString(ui->lineDate->text().trimmed(), "dd.MM.yyyy");
    if (!res.header.date.isValid()) res.header.date = QDate::currentDate();
    if (ui->lineTime) res.header.time = QTime::fromString(ui->lineTime->text().trimmed(), "HH:mm:ss.zzz");
    if (!res.header.time.isValid()) res.header.time = QTime::currentTime();
    res.header.version = 1;
    res.header.commentTextLines << QString::fromUtf8("Сгенерировано программой");

    // записи
    for (int r=0; r<table->rowCount(); ++r) {
        CsvHandler::Record rec;
        rec.x1 = table->item(r,1) ? table->item(r,1)->text().toInt() : 0;
        rec.y1 = table->item(r,2) ? table->item(r,2)->text().toInt() : 0;
        rec.x2 = table->item(r,3) ? table->item(r,3)->text().toInt() : 0;
        rec.y2 = table->item(r,4) ? table->item(r,4)->text().toInt() : 0;
        rec.azimuth = table->item(r,5) ? table->item(r,5)->text().replace(',', '.').toDouble() : 0.0;
        rec.elevation = table->item(r,6) ? table->item(r,6)->text().replace(',', '.').toDouble() : 0.0;
        res.records.push_back(rec);
    }

    CsvHandler handler;
    QString error;
    if (!handler.save(fileName, res, error)) {
        QMessageBox::critical(this, "Ошибка сохранения", error);
        return;
    }
    QMessageBox::information(this, "Сохранение", "Файл сохранён: " + fileName);
}

void MainWindow::addRow() {
    int row = table->rowCount();
    table->blockSignals(true);
    table->insertRow(row);
    {
        auto *it = new QTableWidgetItem(QString::number(row+1));
        it->setFlags(it->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 0, it);
    }
    table->setItem(row, 1, new QTableWidgetItem("0"));
    table->setItem(row, 2, new QTableWidgetItem("0"));
    table->setItem(row, 3, new QTableWidgetItem("0"));
    table->setItem(row, 4, new QTableWidgetItem("0"));
    table->setItem(row, 5, new QTableWidgetItem("0.0"));
    table->setItem(row, 6, new QTableWidgetItem("0.0"));
    table->blockSignals(false);
    drawRectangles();
}

void MainWindow::removeRow() {
    auto selected = table->selectionModel()->selectedRows();
    QList<int> rows;
    for (const QModelIndex &index : selected) rows.append(index.row());
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    table->blockSignals(true);
    for (int r : rows) table->removeRow(r);
    table->blockSignals(false);
    renumberRows();
    drawRectangles();
}

void MainWindow::onTableSelectionChanged() {
    if (isSyncingSelection) return;
    isSyncingSelection = true;
    for (QGraphicsItem* it : scene->selectedItems()) it->setSelected(false);
    auto sel = table->selectionModel()->selectedRows();
    for (const QModelIndex &idx : sel) {
        int row = idx.row();
        for (QGraphicsItem* it : scene->items()) {
            QVariant v = it->data(ROW_ROLE);
            if (v.isValid() && v.toInt() == row) {
                it->setSelected(true);
            }
        }
    }
    isSyncingSelection = false;
}

void MainWindow::onSceneSelectionChanged() {
    if (isSyncingSelection) return;
    isSyncingSelection = true;
    table->selectionModel()->clearSelection();
    QSet<int> rowsToSelect;
    static const int INTERSECT_AREA_ROLE = 2;
    QList<QGraphicsItem*> selected = scene->selectedItems();
    // 1) обычный выбор по элементам
    for (QGraphicsItem* it : selected) {
        QVariant v = it->data(ROW_ROLE);
        if (v.isValid()) rowsToSelect.insert(v.toInt());
    }
    // 2) если выбран оверлей пересечения, выделяем все объекты, фигуры которых пересекают область
    for (QGraphicsItem* it : selected) {
        QVariant a = it->data(INTERSECT_AREA_ROLE);
        if (a.isValid()) {
            QRectF area = a.toRectF();
            for (QGraphicsItem* all : scene->items()) {
                QVariant vr = all->data(ROW_ROLE);
                if (!vr.isValid()) continue;
                QRectF br = all->sceneBoundingRect();
                if (br.intersects(area)) {
                    rowsToSelect.insert(vr.toInt());
                    all->setSelected(true);
                }
            }
        }
    }
    for (int row : rowsToSelect) table->selectRow(row);
    isSyncingSelection = false;
}

void MainWindow::onTableCellChanged(int row, int column) {
    Q_UNUSED(column);
    if (row < 0 || row >= table->rowCount()) return;
    if (!isRedrawing) {
        drawRectangles();
    }
}

void MainWindow::drawRectangles() {
    if (!scene) return;
    if (isRedrawing) return;
    isRedrawing = true;
    table->blockSignals(true);
    qDebug() << "очищение сцены";
    scene->clear();
    struct R { int r; QRectF rect; ObjectType type; };
    QVector<R> rects;
    qDebug() << "рисуем ряд " << table->rowCount();

    // панорама: 3840x512px
    const double PANORAMA_WIDTH = 3840.0;
    const double PANORAMA_HEIGHT = 512.0;
    const double DEG_PER_PX = 360.0 / PANORAMA_WIDTH; // 0.09375 гр/пикс

    for (int row=0; row<table->rowCount(); ++row) {
        bool ok = true;
        int x1 = table->item(row,1) ? table->item(row,1)->text().toInt(&ok) : 0;
        if (!ok) { ok=true; continue; }
        int y1 = table->item(row,2) ? table->item(row,2)->text().toInt(&ok) : 0;
        if (!ok) { ok=true; continue; }
        int x2 = table->item(row,3) ? table->item(row,3)->text().toInt(&ok) : 0;
        if (!ok) { ok=true; continue; }
        int y2 = table->item(row,4) ? table->item(row,4)->text().toInt(&ok) : 0;
        if (!ok) { ok=true; continue; }
        bool okd = true;
        double dAz = table->item(row,5) ? table->item(row,5)->text().replace(',', '.').toDouble(&okd) : 0.0;
        if (!okd) dAz = 0.0;
        okd = true;
        double dEl = table->item(row,6) ? table->item(row,6)->text().replace(',', '.').toDouble(&okd) : 0.0;
        if (!okd) dEl = 0.0;


        if (x1 > x2 || y1 > y2) {
            qDebug() << "рисуем ряд " << row+1 << ": пропуск (start > end: x1=" << x1 << ">x2=" << x2 << " or y1=" << y1 << ">y2=" << y2 << ")";
            continue;
        }


        ObjectType objType = getObjectType(x1, y1, x2, y2);
        

        double dx = dAz / DEG_PER_PX;
        double dy = -dEl / DEG_PER_PX;
        
        QPointF p1 = QPointF(x1 + dx, y1 + dy);
        QPointF p2 = QPointF(x2 + dx, y2 + dy);
        
        auto wrapX = [PANORAMA_WIDTH](double x) -> double {
            while (x < 0) x += PANORAMA_WIDTH;
            while (x >= PANORAMA_WIDTH) x -= PANORAMA_WIDTH;
            return x;
        };

        const double V_PERIOD = 3840.0;
        auto wrapY = [V_PERIOD](double y) -> double {
            double r = std::fmod(y, V_PERIOD);
            if (r < 0) r += V_PERIOD;
            return r;
        };
        auto appendVisibleYSegments = [&](double bx1, double by1, double bx2, double by2) {

            double wy1 = wrapY(by1);
            double wy2 = wrapY(by2);
            auto emitSegment = [&](double segY1, double segY2) {
                double a = qMax(0.0, qMin(PANORAMA_HEIGHT, segY1));
                double b = qMax(0.0, qMin(PANORAMA_HEIGHT, segY2));
                if (a == b) {

                    if (a <= 0.0 || a >= PANORAMA_HEIGHT) return;
                }
                if (a > b) std::swap(a, b);
                if (b <= 0.0 || a >= PANORAMA_HEIGHT) return;
                QRectF rr(bx1, a, bx2 - bx1, b - a);
                rects.append({row, rr, objType});
                qDebug() << "рисуем ряд" << row+1 << " видимый seg (x,y,w,h)=" << rr.x() << rr.y() << rr.width() << rr.height();
            };
            if (wy1 <= wy2) {

                emitSegment(wy1, wy2);
            } else {

                emitSegment(wy1, V_PERIOD);
                emitSegment(0.0, wy2);
            }
        };
        

        double x1_wrapped = wrapX(p1.x());
        double x2_wrapped = wrapX(p2.x());
        

        if (qAbs(x1_wrapped - x2_wrapped) > PANORAMA_WIDTH / 2) {

            QPointF left_p1 = QPointF(x1_wrapped, p1.y());
            QPointF left_p2 = QPointF(PANORAMA_WIDTH, p2.y());
            if (left_p1.x() < left_p2.x()) {

                appendVisibleYSegments(left_p1.x(), left_p1.y(), left_p2.x(), left_p2.y());
            }

            QPointF right_p1 = QPointF(0, p1.y());
            QPointF right_p2 = QPointF(x2_wrapped, p2.y());
            if (right_p1.x() < right_p2.x()) {
                appendVisibleYSegments(right_p1.x(), right_p1.y(), right_p2.x(), right_p2.y());
            }
            continue;
        }
        
        p1.setX(x1_wrapped);
        p2.setX(x2_wrapped);
        
        appendVisibleYSegments(p1.x(), p1.y(), p2.x(), p2.y());
        qDebug() << "рисуем ряд" << row+1 << ": тип" << (objType==ObjectType::Point?"точка":objType==ObjectType::Line?"линия":"прямоугольник")
                 << " dAz=" << dAz << " dEl=" << dEl;
    }

    QSet<int> intersectRows;
    for (int i=0; i<rects.size(); ++i) {
        for (int j=i+1; j<rects.size(); ++j) {
            if (rects[i].rect.intersects(rects[j].rect)) {
                qDebug() << "пересечение: ряды " << rects[i].r+1 << "и" << rects[j].r+1
                         << " area=" << rects[i].rect.intersected(rects[j].rect);
                intersectRows.insert(rects[i].r);
                intersectRows.insert(rects[j].r);
            }
        }
    }

    for (const R &r : rects) {
        QGraphicsItem *item = nullptr;
        QPen pen(Qt::green, 2);
        
        if (r.type == ObjectType::Point) {
            // точка
            QGraphicsLineItem *line1 = new QGraphicsLineItem(r.rect.x()-3, r.rect.y()-3, r.rect.x()+3, r.rect.y()+3);
            QGraphicsLineItem *line2 = new QGraphicsLineItem(r.rect.x()-3, r.rect.y()+3, r.rect.x()+3, r.rect.y()-3);
            line1->setData(ROW_ROLE, r.r);
            line2->setData(ROW_ROLE, r.r);
            line1->setFlag(QGraphicsItem::ItemIsSelectable, true);
            line2->setFlag(QGraphicsItem::ItemIsSelectable, true);
            line1->setPen(pen);
            line2->setPen(pen);
            scene->addItem(line1);
            scene->addItem(line2);
            item = line1;
        }
        else if (r.type == ObjectType::Line) {
            // отрезок
            auto *lineItem = new QGraphicsLineItem(r.rect.x(), r.rect.y(), r.rect.x() + r.rect.width(), r.rect.y() + r.rect.height());
            lineItem->setPen(pen);
            lineItem->setData(ROW_ROLE, r.r);
            lineItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            scene->addItem(lineItem);
            item = lineItem;
        }
        else {
            // прямоугольник
            auto rectItem = scene->addRect(r.rect, pen);
            rectItem->setBrush(Qt::NoBrush);
            item = rectItem;
        }

        if (item) {
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            item->setData(ROW_ROLE, r.r);
            
            if (intersectRows.contains(r.r)) {
                if (auto shape = qgraphicsitem_cast<QAbstractGraphicsShapeItem*>(item)) {
                    shape->setPen(QPen(Qt::red, 2));
                } else if (auto line = qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
                    line->setPen(QPen(Qt::red, 2));
                }
                for (int c=0;c<table->columnCount();++c) {
                    QTableWidgetItem *it = table->item(r.r,c);
                    if (!it) it = new QTableWidgetItem(), table->setItem(r.r,c, it);
                    it->setBackground(Qt::red);
                }
            }
            else {
                for (int c=0;c<table->columnCount();++c) {
                    if (table->item(r.r,c)) table->item(r.r,c)->setBackground(Qt::white);
                }
            }
        }
    }

    static const int INTERSECT_AREA_ROLE = 2;
    for (int i=0; i<rects.size(); ++i) {
        for (int j=i+1; j<rects.size(); ++j) {
            QRectF inter = rects[i].rect.intersected(rects[j].rect);
            if (!inter.isEmpty()) {
                QGraphicsRectItem *over = scene->addRect(inter, QPen(Qt::NoPen), QBrush(QColor(200,0,0,150)));
                over->setZValue(1);
                over->setFlag(QGraphicsItem::ItemIsSelectable, true);
                over->setData(INTERSECT_AREA_ROLE, inter);
            }
        }
    }

    for (QGraphicsItem* it : scene->selectedItems()) {
        QGraphicsRectItem *ri = qgraphicsitem_cast<QGraphicsRectItem*>(it);
        if (ri) ri->setPen(QPen(Qt::blue, 2));
    }

    table->blockSignals(false);
    isRedrawing = false;
}

void MainWindow::renumberRows() {
    for (int i=0;i<table->rowCount();++i) {
        if (!table->item(i,0)) {
            auto *it = new QTableWidgetItem();
            it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            table->setItem(i,0,it);
        }
        table->item(i,0)->setText(QString::number(i+1));
    }
}

bool MainWindow::hasValidationErrors(QString &msg) const {
    struct R { QRectF rect; int row; };
    QVector<R> rects;
    for (int row=0; row<table->rowCount(); ++row) {

        if (!table->item(row,1) || !table->item(row,2) || !table->item(row,3) || !table->item(row,4)) continue;
        QString sx1 = table->item(row,1)->text().trimmed(); if (sx1.isEmpty()) continue;
        QString sy1 = table->item(row,2)->text().trimmed(); if (sy1.isEmpty()) continue;
        QString sx2 = table->item(row,3)->text().trimmed(); if (sx2.isEmpty()) continue;
        QString sy2 = table->item(row,4)->text().trimmed(); if (sy2.isEmpty()) continue;
        bool ok=true; int x1=sx1.toInt(&ok); if(!ok){msg=QString("Строка %1: Xнач не число").arg(row+1);return true;}
        int y1=sy1.toInt(&ok); if(!ok){msg=QString("Строка %1: Yнач не число").arg(row+1);return true;}
        int x2=sx2.toInt(&ok); if(!ok){msg=QString("Строка %1: Xкон не число").arg(row+1);return true;}
        int y2=sy2.toInt(&ok); if(!ok){msg=QString("Строка %1: Yкон не число").arg(row+1);return true;}
        if (x1<0||y1<0||x2<0||y2<0) { msg=QString("Строка %1: отрицательные координаты").arg(row+1); return true; }

        if (x2>3840||x1>=3840||y2>512||y1>=512) { msg=QString("Строка %1: координаты вне 3840x512").arg(row+1); return true; }
        rects.push_back({QRectF(x1,y1,x2-x1,y2-y1), row});

        bool okd=true; table->item(row,5)?table->item(row,5)->text().replace(',', '.').toDouble(&okd):0.0; if(!okd){msg=QString("Строка %1: Азимут не число").arg(row+1);return true;}
        okd=true; table->item(row,6)?table->item(row,6)->text().replace(',', '.').toDouble(&okd):0.0;  if(!okd){msg=QString("Строка %1: Угол не число").arg(row+1);return true;}
    }

    return false;
}

ObjectType MainWindow::getObjectType(int x1, int y1, int x2, int y2) const {
    if (x1 == x2 && y1 == y2) return ObjectType::Point;
    if ((x1 == x2) != (y1 == y2)) return ObjectType::Line;  // только одна сторона
    return ObjectType::Rectangle;
}

void MainWindow::testPanoramaMath() {
    // qDebug() << "=== ТЕСТЫ ПАНОРАМЫ ===";
    
    // const double PANORAMA_WIDTH = 3840.0;
    // auto wrapX = [PANORAMA_WIDTH](double x) -> double {
    //     while (x < 0) x += PANORAMA_WIDTH;
    //     while (x >= PANORAMA_WIDTH) x -= PANORAMA_WIDTH;
    //     return x;
    // };
    
    // qDebug() << "Тест 1 - Циклическое заворачивание:";
    // qDebug() << "wrapX(4000) =" << wrapX(4000) << "(ожидается 160)";
    // qDebug() << "wrapX(-100) =" << wrapX(-100) << "(ожидается 3740)";
    // qDebug() << "wrapX(1920) =" << wrapX(1920) << "(ожидается 1920)";
    // const double DEG_PER_PX = 360.0 / PANORAMA_WIDTH;
    // qDebug() << "Тест 2 - Смещение по азимуту:";
    // qDebug() << "DEG_PER_PX =" << DEG_PER_PX << "(ожидается 0.09375)";
    
    // double testAz = 90.0; // 90 гр
    // double dx = testAz / DEG_PER_PX;
    // qDebug() << "90° смещение =" << dx << "px (ожидается 960)";
    // qDebug() << "Тест 3 - Объект на границе:";
    // int x1 = 3800, y1 = 100, x2 = 3850, y2 = 150; //за право
    // double dAz = 45.0; // 45 гр смещения
    // double dx_test = dAz / DEG_PER_PX;
    // double new_x1 = x1 + dx_test;
    // double new_x2 = x2 + dx_test;
    // qDebug() << "Исходные координаты: x1=" << x1 << " x2=" << x2;
    // qDebug() << "После смещения на 45°: x1=" << new_x1 << " x2=" << new_x2;
    // qDebug() << "После заворачивания: x1=" << wrapX(new_x1) << " x2=" << wrapX(new_x2);
    // qDebug() << "Тест 4 - Объект в слепой зоне:";
    // int y_test = 600; // за 512
    // qDebug() << "Y=" << y_test << " - объект должен быть скрыт (Y >= 512)";
    
    // qDebug() << "=== КОНЕЦ ТЕСТОВ ===";
}
