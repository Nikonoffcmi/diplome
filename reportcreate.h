#ifndef REPORTCREATE_H
#define REPORTCREATE_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

namespace Ui {
class ReportCreate;
}

struct ActData {
    QString productSerial;
    QString batchNumber;
    QString productType;
    QList<double> measurements;
    QList<QString> measurementPoints;
    QString inspectorName;
    QString deviceSerial;
    QDateTime measureDateTime;
    bool qualityStatus;
};

class ReportCreate : public QDialog
{
    Q_OBJECT

public:
    explicit ReportCreate(QWidget *parent = nullptr);
    ~ReportCreate();

    ActData fetchActData(int productId);

private:
    Ui::ReportCreate *ui;
};

#endif // REPORTCREATE_H
