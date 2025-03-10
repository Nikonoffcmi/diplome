#include "datacharts.h"
#include "ui_datacharts.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QtCharts/QChartView>
#include <QVBoxLayout>
#include <QSqlQuery>

DataCharts::DataCharts(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DataCharts)
{
    ui->setupUi(this);
    setupTabs();
}

DataCharts::~DataCharts()
{
    delete ui;
}

void DataCharts::setupTabs() {
    // Создаем контейнер вкладок
    tabWidget = new QTabWidget(this);

    tabWidget->addTab(new Dashboard(), "Аналитика");

    // Настройка главного макета
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);  // Устанавливаем макет для QDialog

    // Дополнительные настройки окна
    setWindowTitle("Анализ защитного слоя");
    resize(800, 600);  // Стартовый размер окна

}


Dashboard::Dashboard(QWidget *parent) : QWidget(parent) {
    setupUI();
    populateComboBoxes();
    updateCharts();
}

void Dashboard::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Комбобоксы для фильтров
    productComboBox = new QComboBox(this);
    productTypeComboBox = new QComboBox(this);
    connect(productComboBox, &QComboBox::currentTextChanged, this, &Dashboard::updateCharts);
    connect(productTypeComboBox, &QComboBox::currentTextChanged, this, &Dashboard::updateCharts);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Изделие:"));
    filterLayout->addWidget(productComboBox);
    filterLayout->addWidget(new QLabel("Тип изделия:"));
    filterLayout->addWidget(productTypeComboBox);

    mainLayout->addLayout(filterLayout);

    // Области для графиков
    timeSeriesChartView = new QChartView();
    barChartView = new QChartView();
    pieChartView = new QChartView();

    QGridLayout *chartsLayout = new QGridLayout();
    chartsLayout->addWidget(timeSeriesChartView, 0, 0);
    chartsLayout->addWidget(barChartView, 0, 1);
    chartsLayout->addWidget(pieChartView, 1, 0, 1, 2);

    mainLayout->addLayout(chartsLayout);
}

void Dashboard::populateComboBoxes() {
    QSqlQuery productQuery("SELECT product_code FROM product");
    while (productQuery.next()) {
        productComboBox->addItem(productQuery.value(0).toString());
    }

    QSqlQuery typeQuery("SELECT name FROM product_type");
    while (typeQuery.next()) {
        productTypeComboBox->addItem(typeQuery.value(0).toString());
    }
}

void Dashboard::updateCharts() {
    createTimeSeriesChart();
    createBarChart();
    createPieChart();
}

void Dashboard::createTimeSeriesChart() {
    QSqlQuery query;
    query.prepare(
        "SELECT m.datetime, m.value_measurement, mp.measuring_point "
        "FROM measurement m "
        "JOIN measuring_point mp ON m.id_measurement_point = mp.id "
        "JOIN product p ON m.id_product = p.id "
        "WHERE p.product_code = ?"
        );
    query.addBindValue(productComboBox->currentText());

    if (!query.exec()) return;

    QChart *chart = new QChart();
    QMap<QString, QLineSeries*> series;

    while (query.next()) {
        QDateTime dt = query.value(0).toDateTime();
        double value = query.value(1).toDouble();
        QString point = query.value(2).toString();

        if (!series.contains(point)) {
            series[point] = new QLineSeries();
            series[point]->setName(point);
        }
        series[point]->append(dt.toMSecsSinceEpoch(), value);
    }

    for (auto *s : series) {
        chart->addSeries(s);
    }

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("dd.MM.yyyy");
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto *s : series) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    chart->setTitle("Изменение толщины во времени");
    timeSeriesChartView->setChart(chart);
}

void Dashboard::createBarChart() {
    QSqlQuery query;
    query.prepare(
        "SELECT mp.measuring_point, AVG(m.value_measurement) "
        "FROM measurement m "
        "JOIN measuring_point mp ON m.id_measurement_point = mp.id "
        "JOIN product p ON m.id_product = p.id "
        "JOIN product_type pt ON p.id_product_type = pt.id "
        "WHERE pt.name = ? "
        "GROUP BY mp.measuring_point"
        );
    query.addBindValue(productTypeComboBox->currentText());

    if (!query.exec()) return;

    QBarSeries *series = new QBarSeries();
    QStringList categories;

    while (query.next()) {
        QString point = query.value(0).toString();
        double avg = query.value(1).toDouble();

        QBarSet *set = new QBarSet(point);
        *set << avg;
        series->append(set);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->setTitle("Средняя толщина по точкам");
    barChartView->setChart(chart);
}

void Dashboard::createPieChart() {
    QSqlQuery query("SELECT quality_protective_layer, COUNT(*) FROM product GROUP BY quality_protective_layer");

    QPieSeries *series = new QPieSeries();
    while (query.next()) {
        QString label = query.value(0).toBool() ? "Качественные" : "Бракованные";
        series->append(label, query.value(1).toInt());
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Соотношение качества продукции");
    pieChartView->setChart(chart);
}
