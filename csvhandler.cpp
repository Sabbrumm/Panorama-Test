#include "csvhandler.h"
#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <QDebug>

CsvHandler::CsvHandler() {}

static bool parseInt(const QString &s, int &out, QString &err, const QString &fieldName) {
    bool ok = false;
    int v = s.toInt(&ok);
    if (!ok) { err = QString("Поле %1 не целое: '%2'").arg(fieldName, s); return false; }
    out = v;
    return true;
}

static bool parseDoublePoint(const QString &s, double &out, QString &err, const QString &fieldName) {

    QString t = s;
    t.replace(',', '.');
    bool ok = false;
    double v = t.toDouble(&ok);
    if (!ok) { err = QString("Поле %1 не число: '%2'").arg(fieldName, s); return false; }
    out = v;
    return true;
}

bool CsvHandler::validateRecord(const Record &rec, QString &outError, int maxWidth, int maxHeight) {
    if (rec.x1 < 0 || rec.y1 < 0 || rec.x2 < 0 || rec.y2 < 0) {
        outError = "Координаты не могут быть отрицательными";
        return false;
    }
    if (rec.x1 >= maxWidth || rec.x2 > maxWidth || rec.y1 >= maxHeight || rec.y2 > maxHeight) {
        outError = QString("Координаты выходят за пределы %1x%2").arg(maxWidth).arg(maxHeight);
        return false;
    }
    if (rec.x2 <= rec.x1 || rec.y2 <= rec.y1) {
        outError = "Нулевая или отрицательная ширина/высота прямоугольника";
        return false;
    }
    return true;
}

bool CsvHandler::load(const QString &filename, Result &outResult, QString &outError) const {
    qDebug() << "CSV загружается:" << filename;
    outResult = Result{};
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) { outError = "Не удалось открыть файл"; return false; }
    QTextStream in(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#endif

    bool seenHeader = false;
    bool seenVersion = false;
    bool seenCount = false;
    bool inData = false;
    int declaredCount = -1;
    int lineNo = 0;

    while (!in.atEnd()) {
        QString raw = in.readLine();
        ++lineNo;
        QString line = raw.trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split(';');
        QString key = parts.value(0).trimmed().toLower();

        if (inData) {

            if (parts.size() < 6) { qDebug() << "CSV ошибка: недостаточно полей в ряду" << lineNo; outError = QString("Строка %1: недостаточно полей").arg(lineNo); return false; }
            Record r;
            QString err;
            if (!parseInt(parts[0], r.x1, err, "XНач")) { qDebug() << "CSV ошибка:" << err << "line" << lineNo; outError = QString("Строка %1: %2").arg(lineNo).arg(err); return false; }
            if (!parseInt(parts[1], r.y1, err, "YНач")) { qDebug() << "CSV ошибка:" << err << "line" << lineNo; outError = QString("Строка %1: %2").arg(lineNo).arg(err); return false; }
            if (!parseInt(parts[2], r.x2, err, "XКон")) { qDebug() << "CSV ошибка:" << err << "line" << lineNo; outError = QString("Строка %1: %2").arg(lineNo).arg(err); return false; }
            if (!parseInt(parts[3], r.y2, err, "YКон")) { qDebug() << "CSV ошибка:" << err << "line" << lineNo; outError = QString("Строка %1: %2").arg(lineNo).arg(err); return false; }
            if (!parseDoublePoint(parts[4], r.azimuth, err, "Азимут")) { qDebug() << "CSV error:" << err << "line" << lineNo; outError = QString("Строка %1: %2").arg(lineNo).arg(err); return false; }
            if (!parseDoublePoint(parts[5], r.elevation, err, "Угол")) { qDebug() << "CSV error:" << err << "line" << lineNo; outError = QString("Строка %1: %2").arg(lineNo).arg(err); return false; }

            if (r.x1 > r.x2) { int temp = r.x1; r.x1 = r.x2; r.x2 = temp; }
            if (r.y1 > r.y2) { int temp = r.y1; r.y1 = r.y2; r.y2 = temp; }

            if (r.x1 < 0 || r.x2 >= 3840 || r.y1 < 0 || r.y2 >= 512) {
                qDebug() << "CSV ошибка: за границами" << lineNo << ":" << r.x1 << r.y1 << r.x2 << r.y2;
                outError = QString("Строка %1: координаты вне диапазона [0,3840)x[0,512)").arg(lineNo);
                return false;
            }

            outResult.records.push_back(r);
            qDebug() << "CSV ряд добавлен:" << r.x1 << r.y1 << r.x2 << r.y2 << r.azimuth << r.elevation;
            continue;
        }

        if (key == "text") {
            outResult.header.commentTextLines << parts.mid(1).join(';');
            continue;
        }
        if (key == "header") {
            if (parts.size() < 4) { outError = QString("Строка %1: некорректный header").arg(lineNo); return false; }
            seenHeader = true;
            QString err;
            if (!parseInt(parts[1], outResult.header.machineNumber, err, "НомерМашины")) { outError = QString("Строка %1: %2").arg(lineNo).arg(err); return false; }
            outResult.header.date = QDate::fromString(parts[2], "dd.MM.yyyy");
            outResult.header.time = QTime::fromString(parts[3], "HH:mm:ss.zzz");
            if (!outResult.header.date.isValid() || !outResult.header.time.isValid()) {
                outError = QString("Строка %1: некорректные дата/время в header").arg(lineNo);
                return false;
            }
            qDebug() << "CSV заголовок:" << outResult.header.machineNumber << outResult.header.date << outResult.header.time;
            continue;
        }
        if (key == "version") {
            if (parts.size() < 2) { outError = QString("Строка %1: некорректный version").arg(lineNo); return false; }
            bool ok=false; int ver = parts[1].toInt(&ok);
            if (!ok) { outError = QString("Строка %1: version не число").arg(lineNo); return false; }
            outResult.header.version = ver;
            seenVersion = true;
            qDebug() << "CSV версия:" << ver;
            continue;
        }
        if (key == "count") {
            if (parts.size() < 2) { outError = QString("Строка %1: некорректный count").arg(lineNo); return false; }
            bool ok=false; declaredCount = parts[1].toInt(&ok);
            if (!ok || declaredCount < 0) { outError = QString("Строка %1: count не число").arg(lineNo); return false; }
            seenCount = true;
            qDebug() << "CSV колво рядов:" << declaredCount;
            continue;
        }
        if (key == "data") {
            inData = true;
            qDebug() << "CSV данные начинаются со строки" << lineNo;
            continue;
        }
    }

    if (!seenHeader) { outError = "Отсутствует секция header"; return false; }
    if (!seenVersion) { outError = "Отсутствует секция version"; return false; }
    if (!seenCount) { outError = "Отсутствует секция count"; return false; }
    if (declaredCount != outResult.records.size()) {
        qDebug() << "CSV не совпадает колво рядов:" << declaredCount << "/" << outResult.records.size();
        outError = QString("Несоответствие count (%1) и числа записей (%2)")
                   .arg(declaredCount).arg(outResult.records.size());
        return false;
    }

    // версия протокола: поддерживаем 1
    if (outResult.header.version != 1) {
        outError = QString("Неподдерживаемая версия протокола: %1. Программа поддерживает только версию 1.").arg(outResult.header.version);
        return false;
    }

    qDebug() << "CSV успешно загружено, рядов:" << outResult.records.size();
    return true;
}

bool CsvHandler::save(const QString &filename, const Result &inResult, QString &outError) const {
    qDebug() << "CSV сохранение началось:" << filename << "рядов:" << inResult.records.size();
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { outError = "Не удалось открыть файл для записи"; return false; }
    QTextStream out(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#endif

    for (const QString &t : inResult.header.commentTextLines) {
        out << "text;" << t << "\n";
    }

    // заголовок
    QDate date = inResult.header.date.isValid() ? inResult.header.date : QDate::currentDate();
    QTime time = inResult.header.time.isValid() ? inResult.header.time : QTime::currentTime();
    out << "header;" << inResult.header.machineNumber << ";"
        << date.toString("dd.MM.yyyy") << ";"
        << time.toString("HH:mm:ss.zzz") << "\n";

    out << "version;" << inResult.header.version << "\n";
    out << "count;" << inResult.records.size() << "\n";
    out << "data\n";

    // данные
    for (const Record &r : inResult.records) {
        QString err;
        if (!validateRecord(r, err)) { outError = QString("Невалидная запись при сохранении: %1").arg(err); f.close(); return false; }
        out << r.x1 << ";" << r.y1 << ";" << r.x2 << ";" << r.y2
            << ";" << QString::number(r.azimuth, 'f', 2)
            << ";" << QString::number(r.elevation, 'f', 2) << "\n";
    }

    f.close();
    return true;
}

bool CsvHandler::autoFixResult(Result &result, QString &outError) {
    Q_UNUSED(outError);
    
    for (auto &record : result.records) {
        if (record.x1 > record.x2) { int temp = record.x1; record.x1 = record.x2; record.x2 = temp; }
        if (record.y1 > record.y2) { int temp = record.y1; record.y1 = record.y2; record.y2 = temp; }
        
        record.x1 = qMax(0, qMin(record.x1, 3839));
        record.x2 = qMax(0, qMin(record.x2, 3839));
        record.y1 = qMax(0, qMin(record.y1, 511));
        record.y2 = qMax(0, qMin(record.y2, 511));
    }
    
    return true;
}
