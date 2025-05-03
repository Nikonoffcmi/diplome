#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adddatafile.h"
#include "comportadd.h"
#include "authdialog.h"
#include "exportdialog.h"

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

void MainWindow::on_action_SaveFile_triggered()
{
    ExportDialog *exportDialog = new ExportDialog(this);
    if (exportDialog->exec() == QDialog::DialogCode::Rejected)
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
    connect(exportAction, &QAction::triggered, this, &MainWindow::on_action_SaveFile_triggered);
    connect(openPortAction, &QAction::triggered, this, &MainWindow::on_action_2_triggered);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    if(admin) {
        QMenu *adminMenu = menuBar->addMenu(tr("&Administration"));
        QAction *usersAction = new QAction(tr("User Management"), this);
        QAction *logsAction = new QAction(tr("View Logs"), this);
        adminMenu->addAction(usersAction);
        adminMenu->addAction(logsAction);
    }

    QMenu *toolsMenu = menuBar->addMenu(tr("&Tools"));
    QAction *settingsAction = new QAction(tr("Settings"), this);
    toolsMenu->addAction(settingsAction);

    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    if(admin) {
        QAction *adminHelp = new QAction(tr("Admin Documentation"), this);
        helpMenu->addAction(adminHelp);
    }
    QAction *aboutAction = new QAction(tr("About"), this);
    helpMenu->addAction(aboutAction);
}
