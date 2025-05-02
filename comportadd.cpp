#include "comportadd.h"
#include "QJsonObject"
#include "ui_comportadd.h"

COMportAdd::COMportAdd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::COMportAdd)
{
    ui->setupUi(this);
    dataModel = new QStandardItemModel(this);
    ui->tableView->setModel(dataModel);
    QStringList fields = {"Номер устройства", "Измерение", "Значение"};
    dataModel->clear();
    dataModel->setHorizontalHeaderLabels(fields);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);



    loadPorts();
    ui->lineEditProduct->installEventFilter(this);
    ui->lineEditProductSerial->installEventFilter(this);
    ui->lineEditInspector->installEventFilter(this);
    ui->cmbPoint->installEventFilter(this);

    m_dataManager.loadData();
    setupCompleters();
    connectSignals();
    _serialPort = nullptr;
    ui->label_10->setText("Отключен");
}

COMportAdd::~COMportAdd()
{
    delete ui;
    if (_serialPort != nullptr) {
        _serialPort->close();
        delete _serialPort;
    }
}

void COMportAdd::loadPorts(){
    foreach (auto &port, QSerialPortInfo::availablePorts()) {
        ui->cmbPorts->addItem(port.portName());
    }
}

void COMportAdd::on_btnOpenPort_clicked()
{

    if (_serialPort != nullptr) {
        _serialPort->close();
        delete _serialPort;
    }
    _serialPort = new QSerialPort(this);
    _serialPort->setPortName(ui->cmbPorts->currentText());
    _serialPort->setBaudRate(QSerialPort::Baud9600);
    _serialPort->setDataBits(QSerialPort::Data8);
    _serialPort->setParity(QSerialPort::NoParity);
    _serialPort->setStopBits(QSerialPort::OneStop);

    if (_serialPort->open(QIODevice::ReadWrite)) {
        QObject::connect(_serialPort, &QSerialPort::readyRead, this, &COMportAdd::readData);
        ui->label_10->setText("Подключен");
    } else {
        QMessageBox::critical(this, "Ошибка порта", "невозможно присоединиться к порту");
    }
}

void COMportAdd::readData(){
    if (_serialPort == nullptr || !_serialPort->isOpen()){
        QMessageBox::critical(this, "Ошибка порта", "порт не подключен для получения данных");
        return;
    }
    if (_struct.command == 0)
        return;
    auto data = _serialPort->readAll();
    // ui->lstMessages->addItem(QString(data));
    processJson(data);
}

void COMportAdd::processJson(const QByteArray &jsonData) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject obj = doc.object();

    QList<QStandardItem*> items;
    items.append(new QStandardItem(obj["seerial_number"].toString()));
    items.append(new QStandardItem(obj["physical-quantity"].toString()));
    items.append(new QStandardItem(QString::number(obj["value"].toDouble())));

    dataModel->appendRow(items);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
}


void COMportAdd::on_btnAddData_clicked()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();

    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбраны сторки из таблицы!");
        return;
    }
    int row = selected[0].row();


    int serial = getDeviceId(ui->tableView->model()->index(row, 0).data().toString());
    QString quantity = ui->tableView->model()->index(row,1).data().toString();
    double value = ui->tableView->model()->index(row,2).data().toDouble();

    if (serial == -1) {
        QMessageBox::critical(this, "Ошибка серийного номера", "Такой серийный номер устройства не существует");
        return;
    }
    if (!checkQuantity(serial, quantity)) {
        QMessageBox::critical(this, "Ошибка измерения", "Такого типа измерения не существует");
        return;
    }

    QString placeStrId = ui->cmbPoint->currentText();
    int placeId = getPlaceId(placeStrId);
    if (placeId == -1) {
        QMessageBox::critical(this, "Ошибка места измерения", "Такое место измерния не найдено");
        return;
    }

    saveToDatabase(value, placeId, serial);
}


bool COMportAdd::validateMeasurement(double value, int placeId) {
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

void COMportAdd::saveToDatabase(double value, int placeId, int serial) {
    QSqlQuery query;
    query.prepare("INSERT INTO measurement (value_measurement, datetime, quality_protective_layer, id_employee, id_product, id_device, id_place_measurement) VALUES (?, LOCALTIMESTAMP, ?, ?, ?, ?, ?)");
    query.addBindValue(value);
    query.addBindValue(validateMeasurement(value, placeId));
    query.addBindValue(searchDataDB("id_person", ui->lineEditInspector->text(), "person", "second_name"));
    query.addBindValue(searchDataDB("id_product", ui->lineEditProductSerial->text(), "product", "product_serial"));
    query.addBindValue(serial);
    query.addBindValue(placeId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }
    else {
        QMessageBox::information(this, "Успех", "Данные добаленны успешно");
    }
}

int COMportAdd::getDeviceId(const QString &serial) {
    QSqlQuery query;
    query.prepare("SELECT id_measuring_device FROM measuring_device WHERE device_serial = ?");
    query.addBindValue(serial);
    query.exec();
    return query.next() ? query.value(0).toInt() : -1;
}

int COMportAdd::getPlaceId(const QString &plase) {
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

int COMportAdd::checkQuantity(const int device_id, const QString &quantity) {
    QSqlQuery query;
    query.prepare("SELECT \"id_JSON\" FROM measurement_characteristics M \
                JOIN measuring_device_model MDM \
                ON MDM.id_measurement_characteristics = M.id_measurement_characteristics \
                JOIN measuring_device MD \
                ON MD.id_measuring_device_model = MDM.id_measuring_device_model \
                WHERE MD.id_measuring_device = ?");
    query.addBindValue(device_id);
    query.exec();
    if (query.next())
        return query.value(0).toString() == quantity ? 1 : 0;
    return 0;
}

qint64 COMportAdd::write(QByteArray data)
{
    if (_serialPort == nullptr || !_serialPort->isOpen()) {
        return -1;
    }
    return _serialPort->write(data);

}

void COMportAdd::on_btnStart_clicked()
{
    if (_serialPort == nullptr || !_serialPort->isOpen()){
        QMessageBox::critical(this, "Ошибка порта", "порт не подключен");
        return;
    }
    _struct.command = 1;
    auto result = sendCommand();
    if (result < 0)
    QMessageBox::critical(this, "Ошибка порта", "Невозмоность записи в порт");
    else
        ui->label_10->setText("Получает данные");
}


void COMportAdd::on_btnStop_clicked()
{
    if (_serialPort == nullptr || !_serialPort->isOpen()){
        QMessageBox::critical(this, "Ошибка порта", "порт не подключен");
        return;
    }
    _struct.command = 0;
    auto result = sendCommand();
    if (result < 0)
        QMessageBox::critical(this, "Ошибка порта", "Невозмоность записи в порт");
    else
        ui->label_10->setText("Подключен");
}

quint64 COMportAdd::sendCommand()
{
    QByteArray ba(reinterpret_cast<char *>(&_struct), sizeof(CommandStruct));
    return write(ba);
}

void COMportAdd::setupCompleters()
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

    // Серийные номера изделий (показываем все по умолчанию)
    ui->lineEditProductSerial->setCompleter(
        createCompleter(m_dataManager.getAllProductSerials()));

    // Изделие
    ui->lineEditProduct->setCompleter(createCompleter(m_dataManager.getProducts()));

    // Проверяющий
    ui->lineEditInspector->setCompleter(createCompleter(m_dataManager.getInspectors()));

    ui->cmbPoint->setEnabled(0);
}

void COMportAdd::connectSignals()
{
    connect(ui->lineEditProduct, &QLineEdit::textChanged, this, &COMportAdd::updateProductSerials);
    connect(ui->lineEditProduct, &QLineEdit::textChanged, this, &COMportAdd::updatePoints);

    // Валидация полей
    connect(ui->lineEditProduct, &QLineEdit::editingFinished, this, &COMportAdd::validateProduct);
    connect(ui->lineEditProductSerial, &QLineEdit::editingFinished, this, &COMportAdd::validateProductSerial);

    // Обработка выбора из списка
    connect(ui->lineEditProductSerial->completer(), QOverload<const QString &>::of(&QCompleter::activated),
            this, &COMportAdd::onProductSerialSelected);
    connect(ui->lineEditInspector, &QLineEdit::editingFinished, this, &COMportAdd::validateInspector);
}

void COMportAdd::updateProductSerials()
{
    QString product = ui->lineEditProduct->text();
    QStringList serials = m_dataManager.getProductSerials(product);

    QCompleter* completer = ui->lineEditProductSerial->completer();
    ContainsFilterProxyModel* proxyModel = static_cast<ContainsFilterProxyModel*>(completer->model());
    QStringListModel* sourceModel = static_cast<QStringListModel*>(proxyModel->sourceModel());
    sourceModel->setStringList(serials);
}

void COMportAdd::updatePoints()
{
    ui->cmbPoint->clear();
    QString product = ui->lineEditProduct->text();
    QStringList points = m_dataManager.getPoints(product);

    ui->cmbPoint->setEnabled(1);
    foreach (auto p, points) {
        ui->cmbPoint->addItem(p);
    }
}

void COMportAdd::validateProduct()
{
    QString text = ui->lineEditProduct->text();
    if (!m_dataManager.getProducts().contains(text))
        ui->lineEditProduct->clear();
}

void COMportAdd::validateProductSerial()
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

void COMportAdd::validateInspector()
{
    QString text = ui->lineEditInspector->text();
    if (!m_dataManager.getInspectors().contains(text, Qt::CaseInsensitive))
        ui->lineEditInspector->clear();
}

void COMportAdd::onProductSerialSelected(const QString &text)
{
    QString product = m_dataManager.productBySerial(text);
    if (!product.isEmpty()) {
        ui->lineEditProduct->setText(product);
        // Обновляем список серийных номеров для выбранного изделия
        updateProductSerials();
    }
}

bool COMportAdd::eventFilter(QObject *obj, QEvent *event)
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
                // Форсируем обновление списка
                comp->setCompletionPrefix(lineEdit->text());
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

QString COMportAdd::searchDataDB(const QString &idName, const QString &data, const QString &dataTable, const QString &dataWhere)
{
    QSqlQuery query;
    QString sqlreq = QString("SELECT "+idName+" FROM " + dataTable + " WHERE " + dataWhere + " = ?");
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

void COMportAdd::on_cmbPorts_currentIndexChanged(int index)
{
    ui->label_10->setText("Отключен");
}


void COMportAdd::on_pushButton_clicked()
{
    close();
}

