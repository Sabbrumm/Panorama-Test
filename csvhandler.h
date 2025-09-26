#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDate>
#include <QTime>

class CsvHandler {
public:
    struct Header {
        int machineNumber = 0;
        QDate date;
        QTime time;
        int version = 1;
        QStringList commentTextLines;
    };

    struct Record {
        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;
        double azimuth = 0.0;
        double elevation = 0.0;
        
        bool isPoint() const { return x1 == x2 && y1 == y2; }
        bool isLine() const { return (x1 == x2) != (y1 == y2); } // только одна сторона нулевая
        bool isRectangle() const { return x1 != x2 && y1 != y2; }
    };

    struct Result {
        Header header;
        QVector<Record> records;
    };

    CsvHandler();

    bool load(const QString &filename, Result &outResult, QString &outError) const;


    bool save(const QString &filename, const Result &inResult, QString &outError) const;


    static bool validateRecord(const Record &rec, QString &outError,
                               int maxWidth = 3840, int maxHeight = 512);
    

    static bool autoFixResult(Result &result, QString &outError);
};

#endif
