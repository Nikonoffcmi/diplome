#include "productmanagementform.h"
#include "qheaderview.h"
#include "qsqlerror.h"
#include "ui_productmanagementform.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QPushButton>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QMessageBox>

ProductManagementForm::ProductManagementForm(bool isAdmin, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProductManagementForm), isAdmin(isAdmin)
    , currentModel(nullptr), currentView(nullptr)
{
    ui->setupUi(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    tabs = new QTabWidget;

    if(isAdmin) {
        setupProductTypeTab();
        // setupBatchTab();
        setupMeasuringPointTab();
        setupPlaceMeasurementTab();
    } else {
        setupProductTypeTab();
    }

    mainLayout->addWidget(tabs);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить");
    QPushButton *deleteButton = new QPushButton("Удалить");
    QPushButton *saveButton = new QPushButton("Сохранить");

    connect(addButton, &QPushButton::clicked, this, &ProductManagementForm::addRecord);
    connect(deleteButton, &QPushButton::clicked, this, &ProductManagementForm::deleteRecord);
    connect(saveButton, &QPushButton::clicked, this, &ProductManagementForm::saveChanges);

    if(!isAdmin) deleteButton->setEnabled(false);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    mainLayout->addLayout(buttonLayout);
}

void ProductManagementForm::setupProductTypeTab() {
    QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("product_type");
    model->setHeaderData(1, Qt::Horizontal, "Тип изделия");
    model->select();

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    modelsMap["Типы изделий"] = model;
    layout->addWidget(view);
    tabs->addTab(tab, "Типы изделий");
}

void ProductManagementForm::setupBatchTab() {
    QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("batch");
    model->setHeaderData(1, Qt::Horizontal, "Номер партии");
    model->select();

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    modelsMap["Партии"] = model;
    layout->addWidget(view);
    tabs->addTab(tab, "Партии");
}

void ProductManagementForm::setupMeasuringPointTab() {
    QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("measuring_point");
    model->setHeaderData(1, Qt::Horizontal, "Точка измерения");
    model->select();

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    modelsMap["Точки измерения"] = model;
    layout->addWidget(view);
    tabs->addTab(tab, "Точки измерения");
}

void ProductManagementForm::setupPlaceMeasurementTab() {
    QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this);
    model->setTable("place_measurement");
    model->setRelation(3, QSqlRelation("product_type", "id_product_type", "name"));
    model->setRelation(4, QSqlRelation("measuring_point", "id_measuring_point", "point"));
    model->select();

    model->setHeaderData(1, Qt::Horizontal, "Верхний предел");
    model->setHeaderData(2, Qt::Horizontal, "Нижний предел");
    model->setHeaderData(3, Qt::Horizontal, "Тип изделия");
    model->setHeaderData(4, Qt::Horizontal, "Точка измерения");

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->setItemDelegate(new QSqlRelationalDelegate(view));
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    modelsMap["Места измерений"] = model;
    layout->addWidget(view);
    tabs->addTab(tab, "Места измерений");
}

void ProductManagementForm::addRecord() {
    QString tabName = tabs->tabText(tabs->currentIndex());
    QSqlTableModel *model = modelsMap.value(tabName, nullptr);

    if(model) {
        model->insertRow(model->rowCount());
    }
}

void ProductManagementForm::deleteRecord() {
    if(!isAdmin) return;

    QString tabName = tabs->tabText(tabs->currentIndex());
    QSqlTableModel *model = modelsMap.value(tabName, nullptr);
    QTableView *view = qobject_cast<QTableView*>(tabs->currentWidget()->findChild<QTableView*>());

    if(model && view && view->currentIndex().isValid()) {
        model->removeRow(view->currentIndex().row());
    }
}

void ProductManagementForm::saveChanges() {
    QString tabName = tabs->tabText(tabs->currentIndex());
    QSqlTableModel *model = modelsMap.value(tabName, nullptr);

    if(model && !model->submitAll()) {
        QMessageBox::critical(this, "Ошибка", model->lastError().text());
    }
}

ProductManagementForm::~ProductManagementForm()
{
    delete ui;
}
