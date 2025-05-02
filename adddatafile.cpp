#include "adddatafile.h"
#include "ui_adddatafile.h"
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"


AddDataFile::AddDataFile(QWidget *parent, int userId)
    : QDialog(parent)
    , ui(new Ui::AddDataFile)
    , tableModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(tableModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_userId = userId;

    ui->lineEditDevice->installEventFilter(this);
    ui->lineEditDeviceSerial->installEventFilter(this);
    ui->lineEditProduct->installEventFilter(this);
    ui->lineEditProductSerial->installEventFilter(this);

    QString fileFilter = " CSV (*.csv);;Text File (*.txt);;Excel (*.xlsx);;All file (*.*) ";
    QString file_name = QFileDialog::getOpenFileName(this, tr("Открыть файл"), "", fileFilter);
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
        QMessageBox::critical(this, tr("Ошибка"), tr("Файл не открывается"));
        return;
    }

    dataModel->clear();
    QTextStream in(&file);
    QString delimiter = ",";

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
        QMessageBox::critical(this, tr("Ошибка"), tr("Файл Ecxel не открывается"));
        return;
    }

    dataModel->clear();
    Worksheet* worksheet = xlsx.currentWorksheet();

    int row = 1;
    int col = 1;
    bool hasHeader = true;

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
}

void AddDataFile::connectSignals()
{
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
}

void AddDataFile::updateInstrumentSerials()
{
    QString instrument = ui->lineEditDevice->text();
    QStringList serials = m_dataManager.getInstrumentSerials(instrument);

    QCompleter* completer = ui->lineEditDeviceSerial->completer();
    ContainsFilterProxyModel* proxyModel = static_cast<ContainsFilterProxyModel*>(completer->model());
    QStringListModel* sourceModel = static_cast<QStringListModel*>(proxyModel->sourceModel());
    sourceModel->setStringList(serials);
}

void AddDataFile::updateProductSerials()
{
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

void AddDataFile::onInstrumentSerialSelected(const QString &text)
{
    QString instrument = m_dataManager.instrumentBySerial(text);
    if (!instrument.isEmpty()) {
        ui->lineEditDevice->setText(instrument);
        updateInstrumentSerials();
    }
}

void AddDataFile::onProductSerialSelected(const QString &text)
{
    QString product = m_dataManager.productBySerial(text);
    if (!product.isEmpty()) {
        ui->lineEditProduct->setText(product);
        updateProductSerials();
    }
}

bool AddDataFile::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        if (auto lineEdit = qobject_cast<QLineEdit*>(obj)) {
            QCompleter *comp = lineEdit->completer();
            if (comp) {
                // Показываем все элементы при пустом поле
                if (lineEdit->text().isEmpty()) {
                    comp->setCompletionPrefix("");
                    comp->complete();
                }
                comp->setCompletionPrefix(lineEdit->text());
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void AddDataFile::on_AccBtn_clicked()
{

    auto MD = searchDataDB("id_measuring_device", ui->lineEditDeviceSerial->text(), "measuring_device", "device_serial");
    auto P = searchDataDB("id_product", ui->lineEditProductSerial->text(), "product", "product_serial");


    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();

    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не выбраны сторки из таблицы!"));
        return;
    }

    foreach (const QModelIndex &index, selected) {
        int row = index.row();
        auto M = ui->tableView->model()->index(row,0).data().toInt();
        auto Date = ui->tableView->model()->index(row,1).data().toString();
        auto Place = getPlaceId(ui->tableView->model()->index(row,2).data().toString());


        if (!insertIntoDatabase(M, Date, P, Place, MD)) {
            QMessageBox::critical(this, tr("Ошибка"), tr("Не получилось добавить данные в базу данных!"));
            return;
        }
    }

    QMessageBox::information(this, tr("Успех"), tr("Данные дабавленные в базу данных!"));

}

bool AddDataFile::insertIntoDatabase(double value, QString Date, QString product, int placeId, QString serial)
{
    QSqlQuery query, query_number;
    query_number.prepare("SELECT MAX(M.measurement_number) FROM measurement M \
                    WHERE M.id_product = ? and M.id_place_measurement = ?");
    query_number.addBindValue(product);
    query_number.addBindValue(placeId);

    if (!query_number.exec()) {
        QMessageBox::critical(this, "Database Error", query_number.lastError().text());
        return false;
    }
    int M_number = 1;
    if (query_number.next()){
        M_number = query_number.value(0).toInt() == 0 ? 1 : query_number.value(0).toInt() + 1;
    }
    query.prepare("INSERT INTO measurement (value_measurement, datetime, quality_protective_layer, "
                  "id_employee, id_product, id_device, id_place_measurement, measurement_number) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(value);
    query.addBindValue(Date);
    query.addBindValue(validateMeasurement(value, placeId));
    query.addBindValue(m_userId);
    query.addBindValue(product);
    query.addBindValue(serial);
    query.addBindValue(placeId);
    query.addBindValue(M_number);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return false;
    }
    return true;
}

QString AddDataFile::searchDataDB(const QString &idName, const QString &data, const QString &dataTable, const QString &dataWhere)
{
    QSqlQuery query;
    QString sqlreq = QString("SELECT "+idName+" FROM " + dataTable + " WHERE " + dataWhere + " = ?");
    query.prepare(sqlreq);

    query.addBindValue(data);
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return {};
    }

    if (query.next()) {
        return query.value(0).toString();
    } else {
        QMessageBox::warning(this, "Warning",
                             QString("Данные не найдены: %1 в таблице %2").arg(data).arg(dataTable));
        return {};
    }
}

int AddDataFile::getPlaceId(const QString &plase) {
    QSqlQuery query;
    query.prepare("SELECT PM.id_place_measurement FROM measuring_point MP \
                JOIN place_measurement PM \
                ON PM.id_measurement_point = MP.id_measuring_point \
                JOIN product_type PT \
                ON PT.id_product_type = PM.id_product_type \
                where mp.point = ? and pt.name = ?;");
    query.addBindValue(plase);
    query.addBindValue(ui->lineEditProduct->text());
    query.exec();
    return query.next() ? query.value(0).toInt() : -1;
}

bool AddDataFile::validateMeasurement(double value, int placeId) {
    QSqlQuery query;
    query.prepare("SELECT lower_limit_thickness, upper_limit_thickness FROM place_measurement WHERE id_place_measurement = ?");
    query.addBindValue(placeId);
    query.exec();
    if (query.next()) {
        double lower = query.value(0).toDouble();
        double upper = query.value(1).toDouble();
        return (value >= lower && value <= upper);
    }
    return false;
}
