#include "adddatafile.h"
#include "ui_adddatafile.h"
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"


AddDataFile::AddDataFile(QWidget *parent, QSqlDatabase db)
    : QDialog(parent)
    , ui(new Ui::AddDataFile)
    , tableModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(tableModel);

    ui->lineEditDevice->installEventFilter(this);
    ui->lineEditDeviceSerial->installEventFilter(this);
    ui->lineEditProduct->installEventFilter(this);
    ui->lineEditProductSerial->installEventFilter(this);
    ui->lineEditInspector->installEventFilter(this);

    QString fileFilter = " CSV (*.csv);;Text File (*.txt);;Excel (*.xlsx);;All file (*.*) ";
    QString file_name = QFileDialog::getOpenFileName(this, "Открыть файл", "", fileFilter);
    if (file_name.isEmpty()) {
        m_operationSuccessful = false;
        return;
    }
    dataModel = new QStandardItemModel(this);
    ui->tableView->setModel(dataModel);

    if (file_name.endsWith(".xlsx")) {
        loadExcelFile(file_name);
    } else {
        loadTextFile(file_name);
    }

    m_dataManager.loadData();
    setupCompleters();
    connectSignals();
}

AddDataFile::~AddDataFile()
{
    delete ui;
}

void AddDataFile::on_CloseBtn_clicked()
{
    close();
}

void AddDataFile::loadTextFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Файл не открывается");
        return;
    }

    dataModel->clear();
    QTextStream in(&file);
    QString delimiter = ",";

    // Автоопределение разделителя
    QString firstLine = in.readLine();
    if (firstLine.contains("\t")) delimiter = "\t";
    else if (firstLine.contains(";")) delimiter = ";";
    else if (firstLine.contains(",")) delimiter = ",";
    in.seek(0);

    int row = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(delimiter);

        if (row == 0) {
            dataModel->setHorizontalHeaderLabels(fields);
        } else {
            QList<QStandardItem*> items;
            for (const QString& field : fields) {
                items.append(new QStandardItem(field));
            }
            dataModel->appendRow(items);
        }
        row++;
    }
    file.close();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
}

void AddDataFile::loadExcelFile(const QString &fileName)
{
    using namespace QXlsx;
    Document xlsx(fileName);
    if (!xlsx.isLoadPackage()) {
        QMessageBox::critical(this, "Ошибка", "Файл Ecxel не открывается");
        return;
    }

    dataModel->clear();
    Worksheet* worksheet = xlsx.currentWorksheet();

    int row = 1;
    int col = 1;
    bool hasHeader = true;

    // Чтение заголовков
    if (hasHeader) {
        QStringList headers;
        auto cell = worksheet->cellAt(row, col);
        while (cell) {
            headers << cell->value().toString();
            col++;
            cell = worksheet->cellAt(row, col);
        }
        dataModel->setHorizontalHeaderLabels(headers);
        row++;
    }

    // Чтение данных
    row = hasHeader ? 2 : 1;
    while (true) {
        auto cell = worksheet->cellAt(row, 1);
        if (!cell) break;

        QList<QStandardItem*> items;
        col = 1;
        while (true) {
            auto cell = worksheet->cellAt(row, col);
            if (!cell) break;

            items.append(new QStandardItem(cell->value().toString()));
            col++;
        }
        dataModel->appendRow(items);
        row++;
    }

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);;
}

void AddDataFile::setupCompleters()
{
//     // Измерительный прибор
//     QCompleter *instrumentCompleter = new QCompleter(m_dataManager.getInstruments(), this);
//     instrumentCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//     ui->lineEditDevice->setCompleter(instrumentCompleter);

//     // Серийный номер прибора
//     QCompleter *instrumentSerialCompleter = new QCompleter(this);
//     instrumentSerialCompleter->setModel(new QStringListModel(m_dataManager.getInstrumentSerials(""), this));
//     ui->lineEditDeviceSerial->setCompleter(instrumentSerialCompleter);

//     // Изделие
//     QCompleter *productCompleter = new QCompleter(m_dataManager.getProducts(), this);
//     productCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//     ui->lineEditProduct->setCompleter(productCompleter);

//     // Серийный номер изделия
//     QCompleter *productSerialCompleter = new QCompleter(this);
//     productSerialCompleter->setModel(new QStringListModel(m_dataManager.getProductSerials(""), this));
//     ui->lineEditProductSerial->setCompleter(productSerialCompleter);

    // Настройка для всех комплетеров
    auto createCompleter = [](const QStringList& data) {
        QCompleter* c = new QCompleter;
        ContainsFilterProxyModel* proxyModel = new ContainsFilterProxyModel(c);
        proxyModel->setSourceModel(new QStringListModel(data, c));
        c->setModel(proxyModel);
        c->setCaseSensitivity(Qt::CaseInsensitive);
        c->setCompletionMode(QCompleter::PopupCompletion);
        return c;
    };

    // Серийные номера приборов (показываем все по умолчанию)
    ui->lineEditDeviceSerial->setCompleter(
        createCompleter(m_dataManager.getAllInstrumentSerials()));

    // Серийные номера изделий (показываем все по умолчанию)
    ui->lineEditProductSerial->setCompleter(
        createCompleter(m_dataManager.getAllProductSerials()));

    // Измерительный прибор
    ui->lineEditDevice->setCompleter(createCompleter(m_dataManager.getInstruments()));

    // Изделие
    ui->lineEditProduct->setCompleter(createCompleter(m_dataManager.getProducts()));

    // Проверяющий
    ui->lineEditInspector->setCompleter(createCompleter(m_dataManager.getInspectors()));
}

void AddDataFile::connectSignals()
{
    // Обновление списков серийных номеров
    connect(ui->lineEditDevice, &QLineEdit::textChanged, this, &AddDataFile::updateInstrumentSerials);
    connect(ui->lineEditProduct, &QLineEdit::textChanged, this, &AddDataFile::updateProductSerials);

    // Валидация полей
    connect(ui->lineEditDevice, &QLineEdit::editingFinished, this, &AddDataFile::validateInstrument);
    connect(ui->lineEditDeviceSerial, &QLineEdit::editingFinished, this, &AddDataFile::validateInstrumentSerial);
    connect(ui->lineEditProduct, &QLineEdit::editingFinished, this, &AddDataFile::validateProduct);
    connect(ui->lineEditProductSerial, &QLineEdit::editingFinished, this, &AddDataFile::validateProductSerial);

    // Обработка выбора из списка
    connect(ui->lineEditDeviceSerial->completer(), QOverload<const QString &>::of(&QCompleter::activated),
            this, &AddDataFile::onInstrumentSerialSelected);
    connect(ui->lineEditProductSerial->completer(), QOverload<const QString &>::of(&QCompleter::activated),
            this, &AddDataFile::onProductSerialSelected);
    connect(ui->lineEditInspector, &QLineEdit::editingFinished, this, &AddDataFile::validateInspector);
}

void AddDataFile::updateInstrumentSerials()
{
    // QString instrument = ui->lineEditDevice->text();
    // QStringList serials = m_dataManager.getInstrumentSerials(instrument);

    // QCompleter *completer = ui->lineEditDeviceSerial->completer();
    // QStringListModel *model = qobject_cast<QStringListModel*>(completer->model());
    // model->setStringList(serials);

    QString instrument = ui->lineEditDevice->text();
    QStringList serials = m_dataManager.getInstrumentSerials(instrument);

    QCompleter* completer = ui->lineEditDeviceSerial->completer();
    ContainsFilterProxyModel* proxyModel = static_cast<ContainsFilterProxyModel*>(completer->model());
    QStringListModel* sourceModel = static_cast<QStringListModel*>(proxyModel->sourceModel());
    sourceModel->setStringList(serials);
}

void AddDataFile::updateProductSerials()
{
//     QString product = ui->lineEditProduct->text();
//     QStringList serials = m_dataManager.getProductSerials(product);

//     QCompleter *completer = ui->lineEditProductSerial->completer();
//     QStringListModel *model = qobject_cast<QStringListModel*>(completer->model());
//     model->setStringList(serials);

    QString product = ui->lineEditProduct->text();
    QStringList serials = m_dataManager.getProductSerials(product);

    QCompleter* completer = ui->lineEditProductSerial->completer();
    ContainsFilterProxyModel* proxyModel = static_cast<ContainsFilterProxyModel*>(completer->model());
    QStringListModel* sourceModel = static_cast<QStringListModel*>(proxyModel->sourceModel());
    sourceModel->setStringList(serials);
}

void AddDataFile::validateInstrument()
{
    QString text = ui->lineEditDevice->text();
    if (!m_dataManager.getInstruments().contains(text))
        ui->lineEditDevice->clear();
}

void AddDataFile::validateInstrumentSerial()
{
    // QString text = ui->lineEditDeviceSerial->text();
    // QString instrument = ui->lineEditDevice->text();
    // QStringList valid = m_dataManager.getInstrumentSerials(instrument);

    // if (!valid.contains(text))
    //     ui->lineEditDeviceSerial->clear();

    QString serial = ui->lineEditDeviceSerial->text();
    QString instrument = ui->lineEditDevice->text();

    bool isValid = instrument.isEmpty()
                       ? m_dataManager.getAllInstrumentSerials().contains(serial)
                       : m_dataManager.getInstrumentSerials(instrument).contains(serial);

    if (!isValid) {
        ui->lineEditDeviceSerial->clear();
        ui->lineEditDevice->clear();
    }
}

void AddDataFile::validateProduct()
{
    QString text = ui->lineEditProduct->text();
    if (!m_dataManager.getProducts().contains(text))
        ui->lineEditProduct->clear();
}

void AddDataFile::validateProductSerial()
{
    // QString text = ui->lineEditProductSerial->text();
    // QString product = ui->lineEditProduct->text();
    // QStringList valid = m_dataManager.getProductSerials(product);

    // if (!valid.contains(text))
    //     ui->lineEditProductSerial->clear();

    QString serial = ui->lineEditProductSerial->text();
    QString product = ui->lineEditProduct->text();

    bool isValid = product.isEmpty()
                       ? m_dataManager.getAllProductSerials().contains(serial)
                       : m_dataManager.getProductSerials(product).contains(serial);

    if (!isValid) {
        ui->lineEditProductSerial->clear();
        ui->lineEditProduct->clear();
    }
}

void AddDataFile::validateInspector()
{
    QString text = ui->lineEditInspector->text();
    if (!m_dataManager.getInspectors().contains(text, Qt::CaseInsensitive))
        ui->lineEditInspector->clear();
}

void AddDataFile::onInstrumentSerialSelected(const QString &text)
{
    // QString instrument = m_dataManager.instrumentBySerial(text);
    // if (!instrument.isEmpty()) {
    //     ui->lineEditDevice->setText(instrument);
    // }

    QString instrument = m_dataManager.instrumentBySerial(text);
    if (!instrument.isEmpty()) {
        ui->lineEditDevice->setText(instrument);
        // Обновляем список серийных номеров для выбранного прибора
        updateInstrumentSerials();
    }
}

void AddDataFile::onProductSerialSelected(const QString &text)
{
    // QString product = m_dataManager.productBySerial(text);
    // if (!product.isEmpty()) {
    //     ui->lineEditProduct->setText(product);
    // }

    QString product = m_dataManager.productBySerial(text);
    if (!product.isEmpty()) {
        ui->lineEditProduct->setText(product);
        // Обновляем список серийных номеров для выбранного изделия
        updateProductSerials();
    }
}

// Переопределение события фокуса для показа полного списка
bool AddDataFile::eventFilter(QObject *obj, QEvent *event)
{
    // if (event->type() == QEvent::FocusIn) {
    //     QLineEdit *lineEdit = qobject_cast<QLineEdit*>(obj);
    //     if (lineEdit && lineEdit->text().isEmpty()) {
    //         QCompleter *comp = lineEdit->completer();
    //         if (comp) {
    //             comp->setCompletionMode(QCompleter::PopupCompletion);
    //             comp->complete();
    //         }
    //     }
    // }
    // return QDialog::eventFilter(obj, event);

    if (event->type() == QEvent::FocusIn) {
        if (auto lineEdit = qobject_cast<QLineEdit*>(obj)) {
            QCompleter *comp = lineEdit->completer();
            if (comp) {
                // Показываем все элементы при пустом поле
                if (lineEdit->text().isEmpty()) {
                    comp->setCompletionPrefix("");
                    comp->complete();
                }
                // Форсируем обновление списка
                comp->setCompletionPrefix(lineEdit->text());
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void AddDataFile::on_AccBtn_clicked()
{
    // Получаем данные из LineEdit
    QVariantList lineEditData = {
        searchDataDB(ui->lineEditDeviceSerial->text(), "measuring_device", "device_code"),
        searchDataDB(ui->lineEditProductSerial->text(), "product", "product_code")
    };

    // Получаем выбранные строки из TableView
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();

    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбраны сторки из таблицы!");
        return;
    }

    // Перебираем выбранные строки
    foreach (const QModelIndex &index, selected) {
        int row = index.row();
        // Получаем данные из 3 колонок таблицы
        QVariantList tableData = {
            ui->tableView->model()->index(row,0).data().toString(),
            ui->tableView->model()->index(row,1).data().toString(),
            searchDataDB(ui->tableView->model()->index(row,2).data().toString(), "measuring_point", "measuring_point")
        };

        // Объединяем данные
        QVariantList allData = lineEditData + tableData;

        // Вставляем в БД
        if (!insertIntoDatabase(allData)) {
            QMessageBox::critical(this, "Ощибка", "Не получилось добавить данные в базу данных!");
            return;
        }
    }

    QMessageBox::information(this, "Успех", "Данные дабавленные в базу данных!");

}

bool AddDataFile::insertIntoDatabase(const QVariantList &data)
{
    QSqlQuery query;
    query.prepare("INSERT INTO measurement"
                  "(id_measuring_device, id_product, value_measurement, datetime, id_measurement_point, measurement_number)"
                  "VALUES (?, ?, ?, ?, ?, 1)");

    for (const QVariant &value : data) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return false;
    }
    return true;
}

QString AddDataFile::searchDataDB(const QString &data, const QString &dataTable, const QString &dataWhere)
{
    QSqlQuery query;
    QString sqlreq = QString("SELECT id FROM %1 WHERE %2 = ?").arg(dataTable).arg(dataWhere);
    query.prepare(sqlreq);

    query.addBindValue(data);
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return {}; // Возвращаем пустую строку вместо nullptr
    }

    // Добавляем проверку наличия результатов
    if (query.next()) {
        return query.value(0).toString();
    } else {
        QMessageBox::warning(this, "Warning",
                             QString("Данные не найдены: %1 в таблице %2").arg(data).arg(dataTable));
        return {};
    }
}
