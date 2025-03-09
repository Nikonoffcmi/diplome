#ifndef DATAANALYZE_H
#define DATAANALYZE_H

#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QFileDialog>

using namespace QXlsx;

namespace Ui {
class DataAnalyze;
}

class DataAnalyze : public QDialog
{
    Q_OBJECT

public:
    explicit DataAnalyze(QWidget *parent = nullptr);
    ~DataAnalyze();

private slots:
    void on_calculateButton_clicked();

    void on_saveButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::DataAnalyze *ui;
    QSortFilterProxyModel* proxyModel;
    QStandardItemModel* sourceModel;

    void refreshStats();
    void loadFilters();
    void addNumericItem(QList<QStandardItem*>& row, const QVariant& value);
    void on_typeFilterCombo_currentIndexChanged(int);
    void on_pointFilterCombo_currentIndexChanged(int);
    void on_totalStatsRadio_toggled(bool);
    void saveToCSV(const QString &filename, const QChar &separator = ';');
    void saveToTXT(const QString &filename);
    void saveToXLSX(const QString &filename);
};

#endif // DATAANALYZE_H
