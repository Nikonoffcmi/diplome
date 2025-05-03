#include "exportdialog.h"
#include "ui_exportdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSqlError>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include "xlsxdocument.h"
#include "xlsxworkbook.h"

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    setupUI();
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);


    QLabel *exportLabel = new QLabel(tr("Экспорт данных"));

    layout->addWidget(exportLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *selectAllButton = new QPushButton(tr("Выбрать все"));
    QPushButton *deselectAllButton = new QPushButton(tr("Снять все"));

    connect(selectAllButton, &QPushButton::clicked, this, &ExportDialog::selectAll);
    connect(deselectAllButton, &QPushButton::clicked, this, &ExportDialog::deselectAll);

    buttonLayout->addWidget(selectAllButton);
    buttonLayout->addWidget(deselectAllButton);

    layout->insertLayout(layout->count() - 2, buttonLayout);

    setupFields();

    QLabel *formatLabel = new QLabel(tr("Формат файла:"));
    formatCombo = new QComboBox;

    formatCombo->addItem("XLSX", "xlsx");
    formatCombo->addItem("TXT", "txt");
    formatCombo->addItem("CSV", "csv");

    layout->addWidget(formatLabel);
    layout->addWidget(formatCombo);

    QPushButton *exportButton = new QPushButton(tr("Экспорт"));
    connect(exportButton, &QPushButton::clicked, this, &ExportDialog::exportData);
    layout->addWidget(exportButton);
}

void ExportDialog::setupFields()
{
    fields = {
        {tr("Значение измерений"), "value_measurement", new QCheckBox(tr("Значение измерений"))},
        {tr("Время измерения"), "datetime", new QCheckBox(tr("Время измерения"))},
        {tr("Номер измерения"), "measurement_number", new QCheckBox(tr("Номер измерения"))},
        {tr("Качество"), "quality_protective_layer", new QCheckBox(tr("Качество"))},
        {tr("Серийный номер прибора"), "device_serial", new QCheckBox(tr("Серийный номер прибора"))},
        {tr("Тип прибора"), "device_type", new QCheckBox(tr("Тип прибора"))},
        {tr("Серийный номер продукта"), "product_serial", new QCheckBox(tr("Серийный номер продукта"))},
        {tr("Тип продукта"), "product_type", new QCheckBox(tr("Тип продукта"))},
        {tr("Точка измерения"), "measurement_point", new QCheckBox(tr("Точка измерения"))},
        {tr("ФИО сотрудника"), "employee_name", new QCheckBox(tr("ФИО сотрудника"))},
        {tr("Подразделение"), "division", new QCheckBox(tr("Подразделение"))},
        {tr("Должность"), "post", new QCheckBox(tr("Должность"))}
    };

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    for (ExportField &field : fields) {
        layout->insertWidget(layout->count(), field.checkbox);
    }
}

QString ExportDialog::getSaveFileName(const QString &suffix)
{
    QString filter;
    if (suffix == "txt") filter = "Text files (*.txt)";
    else if (suffix == "csv") filter = "CSV files (*.csv)";
    else if (suffix == "xlsx") filter = "Excel files (*.xlsx)";

    return QFileDialog::getSaveFileName(this, tr("Сохранить файл"), "", filter);
}

void ExportDialog::exportData()
{
    bool anyChecked = false;
    for (const ExportField &field : fields) {
        if (field.checkbox->isChecked()) {
            anyChecked = true;
            break;
        }
    }
    if (!anyChecked) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Выберите хотя бы одно поле для экспорта."));
        return;
    }

    QStringList selectedColumns, headers;
    for (const ExportField &field : fields) {
        if (field.checkbox->isChecked()) {
            selectedColumns << field.columnName;
            headers << field.displayName;
        }
    }

    QString queryStr =
        "SELECT m.value_measurement, m.datetime, m.measurement_number, "
        "m.quality_protective_layer, md.device_serial, mdm.name AS device_type, "
        "p.product_serial, pt.name AS product_type, mp.point AS measurement_point, "
        "CONCAT(per.name, ' ', per.second_name, ' ', COALESCE(per.middle_name, '')) AS employee_name, "
        "d.name AS division, po.name AS post "
        "FROM measurement m "
        "JOIN measuring_device md ON m.id_device = md.id_measuring_device "
        "JOIN measuring_device_model mdm ON md.id_measuring_device_model = mdm.id_measuring_device_model "
        "JOIN product p ON m.id_product = p.id_product "
        "JOIN product_type pt ON p.id_product_type = pt.id_product_type "
        "JOIN place_measurement pm ON m.id_place_measurement = pm.id_place_measurement "
        "JOIN measuring_point mp ON pm.id_measurement_point = mp.id_measuring_point "
        "JOIN employee e ON m.id_employee = e.id_employee "
        "JOIN person per ON e.id_person = per.id_person "
        "JOIN division d ON e.id_division = d.id_division "
        "JOIN post po ON e.id_post = po.id_post";

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка запроса: ") + query.lastError().text());
        return;
    }

    QVector<QStringList> data;
    while (query.next()) {
        QStringList row;
        for (const QString &col : selectedColumns) {
            row << query.value(col).toString();
        }
        data << row;
    }

    QString suffix = formatCombo->currentData().toString();
    QString fileName = getSaveFileName(suffix);
    if (fileName.isEmpty()) return;

    if (suffix == "txt" || suffix == "csv") {
        QChar delimiter = (suffix == "txt") ? '\t' : ',';
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось открыть файл."));
            return;
        }
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << headers.join(delimiter) << "\n";
        for (const QStringList &row : data) {
            out << row.join(delimiter) << "\n";
        }
        file.close();
    } else if (suffix == "xlsx") {
        QXlsx::Document xlsx;
        for (int col = 0; col < headers.size(); ++col) {
            xlsx.write(1, col + 1, headers[col]);
        }
        for (int row = 0; row < data.size(); ++row) {
            for (int col = 0; col < data[row].size(); ++col) {
                xlsx.write(row + 2, col + 1, data[row][col]);
            }
        }
        if (!xlsx.saveAs(fileName)) {
            QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка сохранения XLSX."));
            return;
        }
    }

    QMessageBox::information(this, tr("Успех"), tr("Экспорт завершен."));
}

void ExportDialog::selectAll()
{
    for (ExportField &field : fields) {
        field.checkbox->setChecked(true);
    }
}

void ExportDialog::deselectAll()
{
    for (ExportField &field : fields) {
        field.checkbox->setChecked(false);
    }
}
