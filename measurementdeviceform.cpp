#include "measurementdeviceform.h"
#include "qformlayout.h"
#include "qlineedit.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "ui_measurementdeviceform.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QPushButton>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QMessageBox>
#include <QHeaderView>
#include <QDateEdit>

MeasurementDeviceForm::MeasurementDeviceForm(bool isAdmin, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MeasurementDeviceForm), isAdmin(isAdmin)
{
    ui->setupUi(this);
    setupUI();
}

void MeasurementDeviceForm::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    tabs = new QTabWidget;


    setupDeviceTab();
    if(isAdmin) {
        setupDeviceModelTab();
        setupManufacturerTab();
        setupMeasurementValueTab();
        setupCharacteristicsTab();
    }

    mainLayout->addWidget(tabs);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить");
    QPushButton *deleteButton = new QPushButton("Удалить");
    QPushButton *saveButton = new QPushButton("Сохранить");

    connect(addButton, &QPushButton::clicked, this, &MeasurementDeviceForm::addDevice);
    connect(saveButton, &QPushButton::clicked, this, &MeasurementDeviceForm::saveChanges);
    connect(deleteButton, &QPushButton::clicked, this, &MeasurementDeviceForm::deleteRecord);

    if(!isAdmin) {
        deleteButton->hide();
        addButton->setText("Добавить прибор");
    }

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    mainLayout->addLayout(buttonLayout);
}

// Реализация вкладок для администратора
void MeasurementDeviceForm::setupManufacturerTab() {
    QWidget *tab = new QWidget;
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("manufacturer");
    model->select();

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    model->setHeaderData(1, Qt::Horizontal, "Производитель");

    tabs->addTab(tab, "Производители");
    (new QVBoxLayout(tab))->addWidget(view);
}

void MeasurementDeviceForm::setupMeasurementValueTab() {
    QWidget *tab = new QWidget;
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("measuring_value");
    model->select();

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    model->setHeaderData(1, Qt::Horizontal, "Измерительная величина");

    tabs->addTab(tab, "Величины");
    (new QVBoxLayout(tab))->addWidget(view);
}

void MeasurementDeviceForm::setupCharacteristicsTab() {
    QWidget *tab = new QWidget;
    QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this);
    model->setTable("measurement_characteristics");
    model->setRelation(1, QSqlRelation("measuring_value", "id_measuring_value", "name"));
    model->select();

    model->setHeaderData(1, Qt::Horizontal, "Название");
    model->setHeaderData(2, Qt::Horizontal, "Вверхний диапозон измерений");
    model->setHeaderData(3, Qt::Horizontal, "Нижний диапозон измерений");
    model->setHeaderData(4, Qt::Horizontal, "Погрешность измерения");
    model->setHeaderData(5, Qt::Horizontal, "Запись в JSON");

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->setItemDelegate(new QSqlRelationalDelegate(view));
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    tabs->addTab(tab, "Характеристики");
    (new QVBoxLayout(tab))->addWidget(view);
}

void MeasurementDeviceForm::setupDeviceModelTab() {
    QWidget *tab = new QWidget;
    QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this);
    model->setTable("measuring_device_model");
    model->setRelation(2, QSqlRelation("manufacturer", "id_manufacturer", "name"));
    model->setRelation(4, QSqlRelation("measurement_characteristics", "id_measurement_characteristics", "error_rate"));
    model->select();

    model->setHeaderData(1, Qt::Horizontal, "Название");
    model->setHeaderData(2, Qt::Horizontal, "Производитель");
    model->setHeaderData(3, Qt::Horizontal, "Вес");
    model->setHeaderData(4, Qt::Horizontal, "Погрешность измерения");

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);
    view->setItemDelegate(new QSqlRelationalDelegate(view));
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    tabs->addTab(tab, "Модели");
    (new QVBoxLayout(tab))->addWidget(view);
}

// Основная вкладка для приборов
void MeasurementDeviceForm::setupDeviceTab() {
    QWidget *tab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(tab);

    deviceModel = new QSqlRelationalTableModel(this);
    deviceModel->setTable("measuring_device");
    deviceModel->setRelation(2, QSqlRelation("measuring_device_model", "id_measuring_device_model", "name"));
    deviceModel->select();

    deviceModel->setHeaderData(1, Qt::Horizontal, "Серийный номер");
    deviceModel->setHeaderData(2, Qt::Horizontal, "Модель");
    deviceModel->setHeaderData(3, Qt::Horizontal, "Дата ввода");
    deviceModel->setHeaderData(4, Qt::Horizontal, "Дата поверки");
    deviceModel->setHeaderData(5, Qt::Horizontal, "Дата ремонта");

    deviceView = new QTableView; // Инициализируем здесь
    deviceView->setModel(deviceModel);
    deviceView->setColumnHidden(0, true);
    deviceView->setItemDelegateForColumn(4, new QSqlRelationalDelegate(deviceView));
    deviceView->setItemDelegateForColumn(5, new QSqlRelationalDelegate(deviceView));

    // Настройка делегатов для дат
    class DateDelegate : public QStyledItemDelegate {
    public:
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override {
            QDateEdit *editor = new QDateEdit(parent);
            editor->setCalendarPopup(true);
            return editor;
        }
    };
    deviceView->setItemDelegateForColumn(3, new DateDelegate());
    deviceView->setItemDelegateForColumn(4, new DateDelegate());
    deviceView->setItemDelegateForColumn(5, new DateDelegate());

    if(!isAdmin) {
        for(int col = 0; col < deviceModel->columnCount(); col++) {
            if(col != 4 && col != 5) { // Разрешаем редактировать только 4 и 5 столбцы
                deviceView->setItemDelegateForColumn(col, new QStyledItemDelegate());
            }
        }
    }

    deviceView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    layout->addWidget(deviceView);
    tabs->addTab(tab, "Приборы");
}

void MeasurementDeviceForm::addDevice() {
    if(!isAdmin) {
        // Для обычных пользователей - только добавление через диалог
        QDialog dialog(this);
        QFormLayout form(&dialog);

        QLineEdit *serialEdit = new QLineEdit;
        QComboBox *modelCombo = new QComboBox;

        // Заполнение комбобокса моделями
        QSqlQuery modelQuery("SELECT id_measuring_device_model, name FROM measuring_device_model");
        while(modelQuery.next()) {
            modelCombo->addItem(modelQuery.value(1).toString(), modelQuery.value(0));
        }

        form.addRow("Серийный номер:", serialEdit);
        form.addRow("Модель:", modelCombo);

        QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        form.addRow(&buttons);

        if(dialog.exec() == QDialog::Accepted) {
            QSqlQuery query;
            query.prepare("INSERT INTO measuring_device (device_serial, id_measuring_device_model) "
                          "VALUES (?, ?)");
            query.addBindValue(serialEdit->text());
            query.addBindValue(modelCombo->currentData());

            if(!query.exec()) {
                QMessageBox::critical(this, "Ошибка", query.lastError().text());
            } else {
                deviceModel->select();
            }
        }
    } else {
        // Для админа - прямое добавление в таблицу
        int row = deviceModel->rowCount();
        if(deviceModel->insertRow(row)) {
            deviceModel->setData(deviceModel->index(row, 3), QDate::currentDate());
        }
    }
}

void MeasurementDeviceForm::saveChanges() {
    if(isAdmin) {
        // Для админа - сохраняем все изменения
        if(!deviceModel->submitAll()) {
            QMessageBox::critical(this, "Ошибка", deviceModel->lastError().text());
        }
    } else {
        // Для пользователя - сохраняем только даты поверки/ремонта
        QModelIndexList indexes = deviceView->selectionModel()->selectedRows();
        foreach(QModelIndex index, indexes) {
            int row = index.row();
            QDate verification = deviceModel->data(deviceModel->index(row, 4)).toDate();
            QDate repair = deviceModel->data(deviceModel->index(row, 5)).toDate();

            QSqlQuery query;
            query.prepare("UPDATE measuring_device SET date_verification = ?, repair_date = ? "
                          "WHERE id_measuring_device = ?");
            query.addBindValue(verification);
            query.addBindValue(repair);
            query.addBindValue(deviceModel->data(deviceModel->index(row, 0)));

            if(!query.exec()) {
                QMessageBox::critical(this, "Ошибка", query.lastError().text());
            }
        }
        deviceModel->select(); // Обновляем данные
    }
}

void MeasurementDeviceForm::deleteRecord() {
    if(!isAdmin) return;

    QModelIndexList indexes = deviceView->selectionModel()->selectedRows();
    foreach(QModelIndex index, indexes) {
        deviceModel->removeRow(index.row());
    }
}

MeasurementDeviceForm::~MeasurementDeviceForm()
{
    delete ui;
}
