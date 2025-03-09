#include "dataanalyze.h"
#include "ui_dataanalyze.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>

DataAnalyze::DataAnalyze(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DataAnalyze)
{
    ui->setupUi(this);

    proxyModel = new QSortFilterProxyModel(this);
    sourceModel = new QStandardItemModel(this);
    proxyModel->setSourceModel(sourceModel);
    ui->tableView->setModel(proxyModel);

    loadFilters();
    refreshStats();
    proxyModel->setSortRole(Qt::DisplayRole);
}

DataAnalyze::~DataAnalyze()
{
    delete ui;
}

void DataAnalyze::refreshStats() {
    QSqlQuery query;
    QString sql =
        "SELECT "
        "pt.name AS product_type,"
        "mp.measuring_point AS measurement_point, \
        ROUND(AVG(m.value_measurement), 2) AS avg_value,\
        MIN(m.value_measurement) AS min_value,\
        MAX(m.value_measurement) AS max_value,\
        ROUND(STDDEV(m.value_measurement), 2) AS std_dev,\
        COUNT(*) AS total_count,\
        SUM(CASE WHEN m.value_measurement > pm.Upper_limit_thickness OR \
        m.value_measurement < pm.lower_limit_thickness THEN 1 ELSE 0 END) AS out_of_limit\
        FROM measurement m\
        JOIN product p ON m.id_product = p.id "
        "JOIN product_type pt ON p.id_product_type = pt.id "
        "JOIN measuring_point mp ON m.id_measurement_point = mp.id "
        "JOIN place_measurement pm ON pt.id = pm.id_product_type AND mp.id = pm.id_measurement_point "
        "WHERE m.datetime BETWEEN :startDate AND :endDate ";

    // Фильтры
    if(ui->typeFilterCombo->currentIndex() > 0) {
        sql += " AND pt.name = '" + ui->typeFilterCombo->currentText() + "'";
    }
    if(ui->pointFilterCombo->currentIndex() > 0 && !ui->totalStatsRadio->isChecked()) {
        sql += " AND mp.measuring_point = '" + ui->pointFilterCombo->currentText() + "'";
    }
    if(ui->qualityCheckBox->isChecked()) {
        sql += " AND p.quality_protective_layer = 1 ";
    }

    // Группировка для разных типов статистики
    if(ui->totalStatsRadio->isChecked()) {
        QRegularExpression regex(
            R"(mp\s*\.\s*measuring_point\s+AS\s+measurement_point\s*,?)",
            QRegularExpression::CaseInsensitiveOption
            );

        // Заменяем совпадения на пустую строку
        sql = sql.replace(regex, "");
        sql += " GROUP BY pt.name";
    } else {
        sql += " GROUP BY pt.name, mp.measuring_point";
    }

    query.prepare(sql);
    query.bindValue(":startDate", ui->startDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":endDate", ui->endDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss"));

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Ошибка запроса: " + query.lastError().text());
        return;
    }


    sourceModel->clear();

    // Заголовки столбцов
    QStringList headers;
    if(ui->totalStatsRadio->isChecked()) {
        headers = {"Тип продукта", "Среднее", "Мин", "Макс",
                   "Ст. отклонение", "Всего", "Брак"};
    } else {
        headers = {"Тип продукта", "Точка измерения", "Среднее", "Мин",
                   "Макс", "Ст. отклонение", "Всего", "Брак"};
    }
    sourceModel->setHorizontalHeaderLabels(headers);

    while(query.next()) {
        QList<QStandardItem*> row;
        row << new QStandardItem(query.value("product_type").toString());

        if(!ui->totalStatsRadio->isChecked()) {
            row << new QStandardItem(query.value("measurement_point").toString());
        }

        addNumericItem(row, query.value("avg_value"));
        addNumericItem(row, query.value("min_value"));
        addNumericItem(row, query.value("max_value"));
        addNumericItem(row, query.value("std_dev"));
        addNumericItem(row, query.value("total_count"));
        addNumericItem(row, query.value("out_of_limit"));

        sourceModel->appendRow(row);
    }

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    if(sourceModel->rowCount() == 0) {
        QMessageBox::information(this, "Информация", "Нет данных для отображения");
        return;
    }
}

void DataAnalyze::addNumericItem(QList<QStandardItem*>& row, const QVariant& value) {
    auto item = new QStandardItem;
    item->setData(value.toDouble(), Qt::DisplayRole);
    item->setTextAlignment(Qt::AlignRight);
    row << item;
}

void DataAnalyze::loadFilters() {
    QSqlQuery query;

    // Загрузка типов продуктов
    query.exec("SELECT DISTINCT name FROM product_type");
    ui->typeFilterCombo->addItem("Все");
    while(query.next()) {
        ui->typeFilterCombo->addItem(query.value(0).toString());
    }

    // Загрузка точек измерения
    query.exec("SELECT DISTINCT measuring_point FROM measuring_point");
    ui->pointFilterCombo->addItem("Все");
    while(query.next()) {
        ui->pointFilterCombo->addItem(query.value(0).toString());
    }
}

void DataAnalyze::on_typeFilterCombo_currentIndexChanged(int) { refreshStats(); }
void DataAnalyze::on_pointFilterCombo_currentIndexChanged(int) { refreshStats(); }
void DataAnalyze::on_totalStatsRadio_toggled(bool) { refreshStats(); }

void DataAnalyze::on_calculateButton_clicked()
{
    refreshStats();
}


void DataAnalyze::on_saveButton_clicked()
{
    QString filter = "CSV (*.csv);;Text (*.txt);;Excel (*.xlsx)";
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить файл",
                                                    QDir::homePath(),
                                                    filter);
    if(filename.isEmpty()) return;

    QFileInfo fi(filename);
    QString ext = fi.suffix().toLower();

    try {
        if(ext == "csv") saveToCSV(filename);
        else if(ext == "txt") saveToTXT(filename);
        else if(ext == "xlsx") saveToXLSX(filename);
        QMessageBox::information(this, "Успех", "Файл успешно сохранен");
    }
    catch(const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", e.what());
    }
}

void DataAnalyze::saveToCSV(const QString &filename, const QChar &separator) {
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw std::runtime_error("Не удалось открыть файл для записи");

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);

    QAbstractItemModel *model = ui->tableView->model();
    const int rowCount = model->rowCount();
    const int colCount = model->columnCount();

    // Заголовки
    QStringList headers;
    for(int c = 0; c < colCount; ++c)
        headers << model->headerData(c, Qt::Horizontal).toString();
    ts << headers.join(separator) << "\n";

    // Данные
    for(int r = 0; r < rowCount; ++r) {
        QStringList row;
        for(int c = 0; c < colCount; ++c)
            row << model->data(model->index(r, c)).toString().replace("\n", " ");
        ts << row.join(separator) << "\n";
    }
}

void DataAnalyze::saveToTXT(const QString &filename) {
    saveToCSV(filename, '\t'); // Используем табуляцию как разделитель
}

void DataAnalyze::saveToXLSX(const QString &filename) {
    QXlsx::Document xlsx;
    QAbstractItemModel *model = ui->tableView->model();

    // Заголовки
    for(int c = 0; c < model->columnCount(); ++c)
        xlsx.write(1, c+1, model->headerData(c, Qt::Horizontal).toString());

    // Данные
    for(int r = 0; r < model->rowCount(); ++r) {
        for(int c = 0; c < model->columnCount(); ++c) {
            xlsx.write(r+2, c+1, model->data(model->index(r, c)).toString());
        }
    }

    if(!xlsx.saveAs(filename))
        throw std::runtime_error("Ошибка сохранения Excel файла");

}

void DataAnalyze::on_pushButton_clicked()
{
    close();
}

