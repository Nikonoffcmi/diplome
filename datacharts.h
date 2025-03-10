#ifndef DATACHARTS_H
#define DATACHARTS_H

#include <QDialog>
#include <QtCharts/QChartView>
#include <QComboBox>
#include <QTabWidget>
#include <QWidget>
#include <QtCharts>

namespace Ui {
class DataCharts;
}

class DataCharts : public QDialog
{
    Q_OBJECT

public:
    explicit DataCharts(QWidget *parent = nullptr);
    ~DataCharts();

private:
    Ui::DataCharts *ui;
    QTabWidget *tabWidget;

    void setupTabs();
};


class Dashboard : public QWidget {
    Q_OBJECT
public:
    explicit Dashboard(QWidget *parent = nullptr);

private slots:
    void updateCharts();

private:
    void createTimeSeriesChart();
    void createBarChart();
    void createPieChart();
    void setupUI();
    void populateComboBoxes();

    QComboBox *productComboBox;
    QComboBox *productTypeComboBox;
    QChartView *timeSeriesChartView;
    QChartView *barChartView;
    QChartView *pieChartView;
};

#endif // DATACHARTS_H
