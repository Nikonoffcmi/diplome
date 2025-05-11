#ifndef REPORTCREATE_H
#define REPORTCREATE_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QTextEdit>
#include <QTextDocument>

namespace Ui {
class ReportCreate;
}

struct MeasurementData {
    double value;
    QString point;
    QString device;
    bool quality;
    QString inspector;
    double max;
    double min;
};

class ReportCreate : public QDialog
{
    Q_OBJECT

public:
    explicit ReportCreate(QWidget *parent = nullptr);
    ~ReportCreate();

private:
    Ui::ReportCreate *ui;
    QTextEdit *htmlPreview;
    QString currentHtml;

    void loadProducts();
    QVector<MeasurementData> getMeasurements(int productId);
    void openHtmlEditor();
    void printPdf();
    void generateHtmlPreview(int productId);
    void saveEditedPdf();
};

#endif // REPORTCREATE_H
