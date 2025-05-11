#include "productform.h"
#include "qformlayout.h"
#include "qheaderview.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "ui_productform.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QMessageBox>
#include <QInputDialog>
#include <QCryptographicHash>

ProductForm::ProductForm(bool isAdmin, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProductForm), isAdmin(isAdmin)
{
    ui->setupUi(this);
    setupUI();
    setupModel();
    setupConnections();
}

ProductForm::~ProductForm()
{
    delete ui;
}

void ProductForm::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    view = new QTableView;
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(view);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("Добавить изделие");
    deleteButton = new QPushButton("Удалить");
    saveButton = new QPushButton("Сохранить");

    if (!isAdmin) {
        deleteButton->setVisible(false);
        saveButton->setVisible(false);
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    } else {
        view->setEditTriggers(QAbstractItemView::DoubleClicked |
                              QAbstractItemView::EditKeyPressed);
    }

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    mainLayout->addLayout(buttonLayout);
    setupSearch(mainLayout);
}

void ProductForm::setupModel() {
    model = new QSqlRelationalTableModel(this);
    model->setTable("product");
    model->setJoinMode(QSqlRelationalTableModel::LeftJoin);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setRelation(2, QSqlRelation("product_type", "id_product_type", "name"));
    model->setRelation(3, QSqlRelation("batch", "id_batch", "batch_number"));
    model->select();

    // Настройка заголовков
    model->setHeaderData(1, Qt::Horizontal, "Серийный номер");
    model->setHeaderData(2, Qt::Horizontal, "Тип изделия");
    model->setHeaderData(3, Qt::Horizontal, "Партия", Qt::DisplayRole);

    view->setModel(model);
    view->setColumnHidden(0, true);
    view->setColumnHidden(3, true);

    view->setItemDelegateForColumn(2, new QSqlRelationalDelegate(view));

    class BatchDelegate : public QSqlRelationalDelegate {
    public:
        void setModelData(QWidget *editor, QAbstractItemModel *model,
                          const QModelIndex &index) const override {
            if (qobject_cast<QComboBox*>(editor)) {
                QVariant data = editor->property("currentData");
                model->setData(index, data.isValid() ? data : QVariant(QVariant::Int));
            } else {
                QSqlRelationalDelegate::setModelData(editor, model, index);
            }
        }
    };
    view->setItemDelegateForColumn(3, new BatchDelegate());
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
}

void ProductForm::setupConnections() {
    connect(addButton, &QPushButton::clicked, this, &ProductForm::addProduct);
    connect(deleteButton, &QPushButton::clicked, this, &ProductForm::deleteProduct);
    connect(saveButton, &QPushButton::clicked, this, &ProductForm::saveChanges);
}

void ProductForm::addProduct() {
    QDialog dialog(this);
    QFormLayout form(&dialog);

    // Поля для ввода
    QLineEdit *serialEdit = new QLineEdit;
    QComboBox *typeCombo = new QComboBox;
    QComboBox *batchCombo = new QComboBox;

    batchCombo->addItem("Не указана", QVariant(QVariant::Int));

    // Заполнение комбобоксов
    QSqlQuery typeQuery("SELECT id_product_type, name FROM product_type");
    while (typeQuery.next()) {
        typeCombo->addItem(typeQuery.value(1).toString(),
                           typeQuery.value(0));
    }

    QSqlQuery batchQuery("SELECT id_batch, batch_number FROM batch");
    while (batchQuery.next()) {
        batchCombo->addItem(batchQuery.value(1).toString(),
                            batchQuery.value(0));
    }

    form.addRow("Серийный номер:", serialEdit);
    form.addRow("Тип изделия:", typeCombo);
    form.addRow("Партия:", batchCombo);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form.addRow(&buttons);

    if (dialog.exec() == QDialog::Accepted) {
        QString serial = serialEdit->text().trimmed();
        if (serial.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Серийный номер не может быть пустым!");
            return;
        }

        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM product WHERE product_serial = ?");
        checkQuery.addBindValue(serial);
        if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Ошибка", "Серийный номер уже существует!");
            return;
        }

        QSqlQuery query;
        query.prepare("INSERT INTO product (product_serial, id_product_type, id_batch) "
                      "VALUES (?, ?, ?)");


        QVariant batchId = batchCombo->currentData();

        query.addBindValue(serialEdit->text());
        query.addBindValue(typeCombo->currentData());
        query.addBindValue(batchId.isNull() ? QVariant(QVariant::Int) : batchId);

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка",
                                  "Не удалось добавить изделие:\n" + query.lastError().text());
        } else {
            model->select();
        }
    }
}

void ProductForm::deleteProduct() {
    if (!isAdmin) return;

    QModelIndex index = view->currentIndex();
    if (index.isValid()) {
        int row = index.row();
        int id = model->data(model->index(row, 0)).toInt();

        QSqlQuery query;
        query.prepare("DELETE FROM product WHERE id_product = ?");
        query.addBindValue(id);

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка",
                                  "Не удалось удалить изделие:\n" + query.lastError().text());
        } else {
            model->select();
        }
    }
}

void ProductForm::saveChanges() {
    if (!isAdmin) return;

    model->database().transaction();

    if (model->submitAll()) {
        model->database().commit();
        QMessageBox::information(this, "Успех", "Изменения сохранены");
    } else {
        model->database().rollback();
        QMessageBox::critical(this, "Ошибка",
                              "Ошибка сохранения:\n" + model->lastError().text());
    }

    model->select();
}

void ProductForm::setupSearch(QVBoxLayout *mainLayout) {
    QLineEdit *searchEdit = new QLineEdit;
    connect(searchEdit, &QLineEdit::textChanged, [this](const QString &text) {
        model->setFilter(QString("product_serial LIKE '%%1%'").arg(text));
        model->select();
    });
    mainLayout->insertWidget(0, searchEdit);
}
