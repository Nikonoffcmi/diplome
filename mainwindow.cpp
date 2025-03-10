#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adddatafile.h"
#include "dataanalyze.h"
#include "datacharts.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    QFile file("./db.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Не удалось открыть файл!";
        return;
    }

    QTextStream in(&file);
    QMap<int, QString> linesMap;
    int lineNumber = 1;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        linesMap.insert(lineNumber++, line);
    }

    file.close();

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(linesMap[1]);
    db.setDatabaseName(linesMap[2]);
    db.setUserName(linesMap[3]);
    db.setPassword(linesMap[4]);

    if (!db.open()){
        qDebug() << db.lastError().text();
        return;
    }

    QSqlQuery query(db);
    query.exec("SELECT M.value_measurement, M.datetime,  MP.measuring_point, P.product_code, PT.name,"
               " CASE WHEN P.quality_protective_layer = 1 THEN 'удовлетворительно' "
               "ELSE 'неудовлетворительно' END AS quality_protective_layer, E.post, CONCAT(E.second_name, ' ', E.first_name)\
        FROM measurement M\
        JOIN product P ON P.id = M.id_product\
        JOIN measuring_point MP ON M.id_measurement_point = MP.id\
        JOIN place_measurement PM ON PM.id_measurement_point = MP.id\
        JOIN product_type PT ON PT.id = P.id_product_type AND PT.id = PM.id_product_type\
        JOIN examiner E ON E.id = P.id_examiner;");

    model = new QSqlQueryModel();
    model->setQuery(std::move(query));
    filModel = new ScanningFilterModel(this);
    filModel->setSourceModel(model);

    ui->tableView->setModel(filModel);
    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    filModel->setHeaderData(0, Qt::Horizontal, "Измерение");
    filModel->setHeaderData(1, Qt::Horizontal, "Время");
    filModel->setHeaderData(2, Qt::Horizontal, "Точка измерения");
    filModel->setHeaderData(3, Qt::Horizontal, "Серийный номер");
    filModel->setHeaderData(4, Qt::Horizontal, "Название изделия");
    filModel->setHeaderData(5, Qt::Horizontal, "Качество");
    filModel->setHeaderData(6, Qt::Horizontal, "Должность");
    filModel->setHeaderData(7, Qt::Horizontal, "Фамилия, имя");



    for (int i = 0; i < filModel->columnCount(); i++) {
        ui->horizontalLayout->addWidget(create_filter_widget_by_scanning_column(i));
    }

    connect(ui->tableView->horizontalHeader(), &QHeaderView::sectionResized,
            this, [this] (int index, int old_size, int new_size)
            {
                Q_UNUSED(old_size);
                auto widget = ui->horizontalLayout->itemAt(index)->widget();
                widget->setMaximumWidth(new_size - 5);
            });
}

MainWindow::~MainWindow()
{
    db.close();
    delete model;
    delete filModel;
    delete ui;
}

QWidget *MainWindow::create_filter_widget_by_scanning_column(const int &column) const
{
    auto widget = new QLineEdit();
    widget->setValidator(new QRegularExpressionValidator(QRegularExpression(".*")));

    // Подключаем сигнал textChanged к слоту set_filter
    connect(widget, &QLineEdit::textChanged, this, [this, column](const QString &text) {
        filModel->set_filter(column, text);
    });
    return widget;
}


// Конструктор
ScanningFilterModel::ScanningFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

// Установка фильтра для конкретного столбца
void ScanningFilterModel::set_filter(int column, const QString &filter)
{
    m_filters[column] = filter; // Сохраняем фильтр для столбца
    invalidateFilter();         // Обновляем фильтр
}

// Метод фильтрации строк
bool ScanningFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // Проходим по всем столбцам, для которых заданы фильтры
    for (auto it = m_filters.constBegin(); it != m_filters.constEnd(); ++it) {
        int column = it.key();
        QString filter = it.value();

        // Если фильтр пуст, пропускаем проверку для этого столбца
        if (filter.isEmpty()) {
            continue;
        }

        // Получаем данные из столбца
        QModelIndex index = sourceModel()->index(sourceRow, column, sourceParent);
        QString data = sourceModel()->data(index).toString();

        // Если данные не соответствуют фильтру, строка не отображается
        if (!data.contains(filter, Qt::CaseInsensitive)) {
            return false;
        }
    }

    // Если все фильтры пройдены, строка отображается
    return true;
}

void MainWindow::on_action_triggered()
{
    AddDataFile* addDataFileDialog = new AddDataFile(this);
    addDataFileDialog->show();

}



void MainWindow::on_pushButton_clicked()
{
    close();
}


void MainWindow::on_action_4_triggered()
{
    DataAnalyze* DADialog = new DataAnalyze(this);
    DADialog->show();
}


void MainWindow::on_action_5_triggered()
{
    DataCharts * DCDialog = new DataCharts(this);
    DCDialog->show();
}

