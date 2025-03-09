#include "adddatafile.h"
#include "ui_adddatafile.h"
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"

using namespace QXlsx;

AddDataFile::AddDataFile(QWidget *parent, QSqlDatabase db)
    : QDialog(parent)
    , ui(new Ui::AddDataFile)
{
    ui->setupUi(this);

    ui->lineEditDevice->installEventFilter(this);
    ui->lineEditDeviceSerial->installEventFilter(this);
    ui->lineEditProduct->installEventFilter(this);
    ui->lineEditProductSerial->installEventFilter(this);
    ui->lineEditInspector->installEventFilter(this);

    QString fileFilter = "All file (*.*) ;; Text File (*.txt) ;; Excel (*.xlsx) ;; CSV (*.csv)";
    QString file_name = QFileDialog::getOpenFileName(this, "Открыть файл", "", fileFilter);
    if (file_name == nullptr) {
        m_operationSuccessful = false;
        return;
    }
    excelModel = new QStandardItemModel(this);
    ui->tableView->setModel(excelModel);

    // Загружаем данные из Excel-файла
    loadDataFromExcel(file_name);

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

void AddDataFile::loadDataFromExcel(const QString &filePath) {
    // Открываем Excel-файл
    Document xlsx(filePath);
    if (!xlsx.load()) {
        qWarning() << "Failed to load file:" << filePath;
        return;
    }

    // Получаем активный лист
    QString sheetName = xlsx.sheetNames().first();
    xlsx.selectSheet(sheetName);

    // Читаем данные из листа
    int rowCount = 0;
    int colCount = 0;

    // Определяем количество строк и столбцов
    while (!xlsx.read(rowCount + 1, 1).isNull()) rowCount++;
    while (!xlsx.read(1, colCount + 1).isNull()) colCount++;

    // Устанавливаем заголовки столбцов
    for (int col = 0; col < colCount; ++col) {
        QVariant header = xlsx.read(1, col + 1);
        excelModel->setHorizontalHeaderItem(col, new QStandardItem(header.toString()));
    }

    // Заполняем модель данными
    for (int row = 1; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            QVariant cellData = xlsx.read(row + 1, col + 1);
            QStandardItem *item = new QStandardItem(cellData.toString());
            excelModel->setItem(row - 1, col, item);
        }
    }
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
