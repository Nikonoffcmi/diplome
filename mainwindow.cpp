#include "mainwindow.h"
#include "measurementdeviceform.h"
#include "productform.h"
#include "productmanagementform.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"
#include "adddatafile.h"
#include "comportadd.h"
#include "authdialog.h"
#include "exportdialog.h"
#include "reportcreate.h"
#include "employeeform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    QFile file("./db.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << tr("Не удалось открыть файл!");
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

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(linesMap[1]);
    db.setDatabaseName(linesMap[2]);
    db.setUserName(linesMap[3]);
    db.setPassword(linesMap[4]);

    if (!db.open()){
        qDebug() << db.lastError().text();
        return;
    }

    AuthDialog *authD = new AuthDialog(this, &admin, &userId);
    if (authD->exec() != QDialog::DialogCode::Accepted)
        QTimer::singleShot(0, this, SLOT(close()));

    QSqlQuery query(db);


    if (!query.exec("SELECT "
                "M.value_measurement, "
                "M.datetime + interval '7 hours' AS datetime, "
                "MP.point AS measuring_point, "
                "P.product_serial AS product_code, "
                "PT.name AS product_name, "
                "CASE WHEN M.quality_protective_layer THEN 'удовлетворительно' "
                "ELSE 'неудовлетворительно' END AS quality_protective_layer, "
                "Post.name AS post, "
                "CONCAT(E.second_name, ' ', E.name) AS examiner_full_name "
                "FROM measurement M "
                "JOIN product P ON M.id_product = P.id_product "
                "JOIN place_measurement PLM ON M.id_place_measurement = PLM.id_place_measurement "
                "JOIN measuring_point MP ON PLM.id_measurement_point = MP.id_measuring_point "
                "JOIN product_type PT ON P.id_product_type = PT.id_product_type "
                "JOIN employee Emp ON M.id_employee = Emp.id_employee "
                "JOIN person E ON Emp.id_person = E.id_person "
                "JOIN post Post ON Emp.id_post = Post.id_post "
                "ORDER BY datetime DESC;")) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
        return;
    }

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
    filModel->setHeaderData(3, Qt::Horizontal, "Серийный номер изделия");
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

    setupMenu();
    ui->retranslateUi(this);
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
    delete model;
    delete filModel;
    delete ui;
}

QWidget *MainWindow::create_filter_widget_by_scanning_column(const int &column) const
{
    auto widget = new QLineEdit();
    widget->setValidator(new QRegularExpressionValidator(QRegularExpression(".*")));

    connect(widget, &QLineEdit::textChanged, this, [this, column](const QString &text) {
        filModel->set_filter(column, text);
    });
    return widget;
}


ScanningFilterModel::ScanningFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void ScanningFilterModel::set_filter(int column, const QString &filter)
{
    m_filters[column] = filter;
    invalidateFilter();
}

bool ScanningFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    for (auto it = m_filters.constBegin(); it != m_filters.constEnd(); ++it) {
        int column = it.key();
        QString filter = it.value();

        if (filter.isEmpty()) {
            continue;
        }

        QModelIndex index = sourceModel()->index(sourceRow, column, sourceParent);
        QString data = sourceModel()->data(index).toString();

        if (!data.contains(filter, Qt::CaseInsensitive)) {
            return false;
        }
    }

    return true;
}

void MainWindow::on_action_triggered()
{
    AddDataFile* addDataFileDialog = new AddDataFile(this, userId);
    if (addDataFileDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}



void MainWindow::on_pushButton_clicked()
{
    close();
}

void MainWindow::on_action_2_triggered()
{
    COMportAdd *COMPDialog = new COMportAdd(this, userId);
    if (COMPDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::on_actionSaveFileTriggered()
{
    ExportDialog *exportDialog = new ExportDialog(this);
    if (exportDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::on_actionReportPDFTriggered()
{
    ReportCreate *reportDialog = new ReportCreate(this);
    if (reportDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::on_actionHTMLTriggered()
{

}

void MainWindow::on_actionEmployeeTriggered()
{
    EmployeeForm *employeeDialog = new EmployeeForm(admin, userId, this);
    if (employeeDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::on_actionProductTypeTriggered()
{
    ProductManagementForm *productDialog = new ProductManagementForm(admin, this);
    if (productDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::on_actionProductTriggered()
{
    ProductForm *productDialog = new ProductForm(admin, this);
    if (productDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::on_actionDeviceTriggered()
{
    MeasurementDeviceForm *deviceDialog = new MeasurementDeviceForm(admin, this);
    if (deviceDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::on_actionSettingsTriggered()
{
    SettingsDialog *settingsDialog = new SettingsDialog(this);
    if (settingsDialog->exec() == QDialog::DialogCode::Rejected)
        refreshScreen();
}

void MainWindow::refreshScreen() {
    QSqlQuery query(db);
    query.exec("SELECT "
               "M.value_measurement, "
               "M.datetime + interval '7 hours' AS datetime, "
               "MP.point AS measuring_point, "
               "P.product_serial AS product_code, "
               "PT.name AS product_name, "
               "CASE WHEN M.quality_protective_layer THEN 'удовлетворительно' "
               "ELSE 'неудовлетворительно' END AS quality_protective_layer, "
               "Post.name AS post, "
               "CONCAT(E.second_name, ' ', E.name) AS examiner_full_name "
               "FROM measurement M "
               "JOIN product P ON M.id_product = P.id_product "
               "JOIN place_measurement PLM ON M.id_place_measurement = PLM.id_place_measurement "
               "JOIN measuring_point MP ON PLM.id_measurement_point = MP.id_measuring_point "
               "JOIN product_type PT ON P.id_product_type = PT.id_product_type "
               "JOIN employee Emp ON M.id_employee = Emp.id_employee "
               "JOIN person E ON Emp.id_person = E.id_person "
               "JOIN post Post ON Emp.id_post = Post.id_post "
               "ORDER BY datetime DESC;");

    model->setQuery(std::move(query));
    ui->retranslateUi(this);
}

void MainWindow::setupMenu() {
    QMenuBar *menuBar = this->menuBar();
    menuBar->clear();

    QMenu *fileMenu = menuBar->addMenu(tr("Файл"));
    QAction *openFileAction = new QAction(tr("Открыть файл"), this);
    QAction *openPortAction = new QAction(tr("Загрузить с устройства"), this);
    QAction *exportAction = new QAction(tr("Сохранить"), this);
    QAction *exitAction = new QAction(tr("Выход"), this);
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(openPortAction);
    fileMenu->addAction(exportAction);
    fileMenu->addAction(exitAction);
    connect(openFileAction, &QAction::triggered, this, &MainWindow::on_action_triggered);
    connect(exportAction, &QAction::triggered, this, &MainWindow::on_actionSaveFileTriggered);
    connect(openPortAction, &QAction::triggered, this, &MainWindow::on_action_2_triggered);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);



    QMenu *booksMenu = menuBar->addMenu(tr("Справочники"));
    QAction *EmployeeAction = new QAction(tr("Сотрудники"), this);
    QAction *productAction = new QAction(tr("Изделия"), this);
    QAction *deviceAction = new QAction(tr("Приборы"), this);


    booksMenu->addAction(EmployeeAction);
    booksMenu->addAction(productAction);

    if(admin) {
        QAction *productTypeAction = new QAction(tr("Тип Изделия"), this);
        booksMenu->addAction(productTypeAction);
        connect(productTypeAction, &QAction::triggered, this, &MainWindow::on_actionProductTypeTriggered);
    }

    booksMenu->addAction(deviceAction);

    connect(EmployeeAction, &QAction::triggered, this, &MainWindow::on_actionEmployeeTriggered);
    connect(productAction, &QAction::triggered, this, &MainWindow::on_actionProductTriggered);
    connect(deviceAction, &QAction::triggered, this, &MainWindow::on_actionDeviceTriggered);


    QMenu *reportMenu = menuBar->addMenu(tr("Отчет"));
    QAction *pdfAction = new QAction(tr("формат pdf"), this);
    QAction *htmlAction = new QAction(tr("Редактировать HTML"), this);

    reportMenu->addAction(pdfAction);

    connect(pdfAction, &QAction::triggered, this, &MainWindow::on_actionReportPDFTriggered);
    connect(htmlAction, &QAction::triggered, this, &MainWindow::on_actionHTMLTriggered);




    QMenu *toolsMenu = menuBar->addMenu(tr("Средства"));
    QAction *settingsAction = new QAction(tr("Настройки"), this);

    toolsMenu->addAction(settingsAction);

    connect(settingsAction, &QAction::triggered, this, &MainWindow::on_actionSettingsTriggered);




    QMenu *helpMenu = menuBar->addMenu(tr("Справка"));

    QAction *aboutAction = new QAction(tr("О программе"), this);
    QAction *toolAction = new QAction(tr("Вызов справки"), this);
    helpMenu->addAction(toolAction);
    helpMenu->addAction(aboutAction);
}
