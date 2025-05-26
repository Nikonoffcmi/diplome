#include "employeeform.h"
#include "qheaderview.h"
#include "qlineedit.h"
#include "qsqlrecord.h"
#include "ui_employeeform.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QTableView>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QPushButton>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

EmployeeForm::EmployeeForm(bool isAdmin, int userId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EmployeeForm)
    , isAdmin(isAdmin), userId(userId)
{
    ui->setupUi(this);


    if (isAdmin) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QTabWidget *tabs = new QTabWidget;

        QWidget *employeeTab = new QWidget;
        setupEmployeeTab(employeeTab);
        tabs->addTab(employeeTab, "Сотрудники");

        QWidget *divisionTab = new QWidget;
        setupDivisionTab(divisionTab);
        tabs->addTab(divisionTab, "Подразделения");

        QWidget *postTab = new QWidget;
        setupPostTab(postTab);
        tabs->addTab(postTab, "Должности");

        layout->addWidget(tabs);
    } else {
        setupUserForm();
    }
}

EmployeeForm::~EmployeeForm()
{
    delete ui;
}

void EmployeeForm::setupPostTab(QWidget *tab) {
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("post");
    model->select();

    // Настройка заголовков
    model->setHeaderData(1, Qt::Horizontal, "Название должности");

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);

    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    layout->addWidget(view);

    // Кнопки управления (аналогично подразделениям)
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить");
    QPushButton *deleteButton = new QPushButton("Удалить");
    QPushButton *saveButton = new QPushButton("Сохранить");

    connect(addButton, &QPushButton::clicked, [model]() {
        model->insertRow(model->rowCount());
    });

    connect(deleteButton, &QPushButton::clicked, [model, view]() {
        model->removeRow(view->currentIndex().row());
    });

    connect(saveButton, &QPushButton::clicked, [model]() {
        if (!model->submitAll()) {
            QMessageBox::critical(nullptr, "Ошибка", model->lastError().text());
        }
    });

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    layout->addLayout(buttonLayout);
}

void EmployeeForm::setupEmployeeTab(QWidget *tab) {
    QVBoxLayout *layout = new QVBoxLayout(tab);

    // Создаем кастомный запрос с объединением таблиц
    model = new QSqlRelationalTableModel(this);
    model->setTable("employee");
    model->setRelation(3, QSqlRelation("post", "id_post", "name"));
    model->setRelation(4, QSqlRelation("division", "id_division", "name"));
    model->setJoinMode(QSqlRelationalTableModel::LeftJoin);

    // Создаем представление с объединенными данными
    model->setQuery(QSqlQuery(
        "SELECT e.id_employee, p.name, p.second_name, p.middle_name, "
        "d.name AS division, pt.name AS post, e.login, e.password, e.admin "
        "FROM employee e "
        "LEFT JOIN person p ON e.id_person = p.id_person "
        "LEFT JOIN division d ON e.id_division = d.id_division "
        "LEFT JOIN post pt ON e.id_post = pt.id_post"
        ));

    // Настраиваем названия столбцов
    model->setHeaderData(1, Qt::Horizontal, "Имя");
    model->setHeaderData(2, Qt::Horizontal, "Фамилия");
    model->setHeaderData(3, Qt::Horizontal, "Отчество");
    model->setHeaderData(4, Qt::Horizontal, "Подразделение");
    model->setHeaderData(5, Qt::Horizontal, "Должность");
    model->setHeaderData(6, Qt::Horizontal, "Логин");
    model->setHeaderData(7, Qt::Horizontal, "Пароль");
    model->setHeaderData(8, Qt::Horizontal, "Админ");


    view = new QTableView;
    view->setModel(model);

    // Скрываем колонку с ID
    view->setColumnHidden(0, true);

    view->setColumnHidden(6, true);
    view->setColumnHidden(7, true);
    view->setColumnHidden(8, true);

    // Настраиваем делегаты
    view->setItemDelegateForColumn(4, new QSqlRelationalDelegate(view)); // Для подразделения
    view->setItemDelegateForColumn(5, new QSqlRelationalDelegate(view)); // Для должности


    // Для пароля используем специальный делегат
    class PasswordDelegate : public QStyledItemDelegate {
    public:
        QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setEchoMode(QLineEdit::Password);
            return editor;
        }
    };
    view->setItemDelegateForColumn(7, new PasswordDelegate());

    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    layout->addWidget(view);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить");
    QPushButton *deleteButton = new QPushButton("Удалить");
    QPushButton *saveButton = new QPushButton("Сохранить");

    connect(addButton, &QPushButton::clicked, this, &EmployeeForm::addEmployee);
    connect(deleteButton, &QPushButton::clicked, this, &EmployeeForm::deleteEmployee);
    connect(saveButton, &QPushButton::clicked, this, &EmployeeForm::saveChanges);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    layout->addLayout(buttonLayout);
}

void EmployeeForm::updatePersonRecord(int personId, const QString &name,
                                      const QString &secondName, const QString &middleName) {
    QSqlQuery query;
    query.prepare("UPDATE person SET name = ?, second_name = ?, middle_name = ? "
                  "WHERE id_person = ?");
    query.addBindValue(name);
    query.addBindValue(secondName);
    query.addBindValue(middleName);
    query.addBindValue(personId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Ошибка обновления ФИО: " + query.lastError().text());
    }
}

void EmployeeForm::setupUserForm() {
    QWidget *formWidget = new QWidget(this);
    QFormLayout *form = new QFormLayout(formWidget);

    QLineEdit *loginEdit = new QLineEdit;
    QLineEdit *passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);

    QSqlQuery query;
    query.prepare("SELECT login, password FROM employee WHERE id_employee = ?");
    query.addBindValue(userId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", query.lastError().text());
        return;
    }

    if (query.next()) {
        loginEdit->setText(query.value(0).toString());
        passwordEdit->setText(query.value(1).toString());
    } else {
        QMessageBox::warning(this, "Внимание", "Пользователь не найден!");
    }

    QPushButton *saveButton = new QPushButton("Сохранить");
    connect(saveButton, &QPushButton::clicked, [=]() {
        // Обновление данных в БД
        QSqlQuery updateQuery;
        updateQuery.prepare(
            "UPDATE employee SET login = ?, password = ? "
            "WHERE id_employee = ?"
            );
        updateQuery.addBindValue(loginEdit->text());

        QString newPassword = passwordEdit->text();
        if (newPassword != "********") {
            updateQuery.addBindValue(QCryptographicHash::hash(
                                         newPassword.toUtf8(),
                                         QCryptographicHash::Sha256
                                         ).toHex());
        } else {
            updateQuery.addBindValue(query.value(1));
        }

        updateQuery.addBindValue(userId);

        if (!updateQuery.exec()) {
            QMessageBox::critical(this, "Ошибка", updateQuery.lastError().text());
        } else {
            QMessageBox::information(this, "Успех", "Данные обновлены!");
        }
    });

    form->addRow("Логин:", loginEdit);
    form->addRow("Пароль:", passwordEdit);
    form->addRow(saveButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(formWidget);
    this->setLayout(mainLayout);
}

void EmployeeForm::addEmployee() {
    QDialog dialog(this);
    QFormLayout form(&dialog);

    QComboBox *personCombo = new QComboBox;
    QSqlQuery personQuery("SELECT id_person, name FROM person");
    while (personQuery.next()) {
        personCombo->addItem(
            personQuery.value(1).toString(),
            personQuery.value(0)
            );
    }

    QComboBox *postCombo = new QComboBox;
    QSqlQuery postQuery("SELECT id_post, name FROM post");
    while (postQuery.next()) {
        postCombo->addItem(
            postQuery.value(1).toString(),
            postQuery.value(0)
            );
    }

    QComboBox *divisionCombo = new QComboBox;
    QSqlQuery divisionQuery("SELECT id_division, name FROM division");
    while (divisionQuery.next()) {
        divisionCombo->addItem(
            divisionQuery.value(1).toString(),
            divisionQuery.value(0)
            );
    }

    QLineEdit *login = new QLineEdit;
    QLineEdit *password = new QLineEdit;
    password->setEchoMode(QLineEdit::Password);

    form.addRow("Человек:", personCombo);
    form.addRow("Должность:", postCombo);
    form.addRow("Подразделение:", divisionCombo);
    form.addRow("Логин:", login);
    form.addRow("Пароль:", password);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form.addRow(&buttons);

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery query;
        query.prepare(
            "INSERT INTO employee (id_person, id_post, id_division, login, password) "
            "VALUES (?, ?, ?, ?, ?)"
            );
        query.addBindValue(personCombo->currentData());
        query.addBindValue(postCombo->currentData());
        query.addBindValue(divisionCombo->currentData());
        query.addBindValue(login->text());
        query.addBindValue(QCryptographicHash::hash(
                               password->text().toUtf8(),
                               QCryptographicHash::Sha256
                               ).toHex());

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", query.lastError().text());
        } else {
            model->select();
        }
    }
}

void EmployeeForm::deleteEmployee() {
    QModelIndex index = view->currentIndex();
    if (index.isValid()) {
        model->removeRow(index.row());
    }
}

void EmployeeForm::setupDivisionTab(QWidget *tab) {
    QVBoxLayout *layout = new QVBoxLayout(tab);

    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("division");
    model->select();

    // Настройка заголовков
    model->setHeaderData(1, Qt::Horizontal, "Название подразделения");

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setColumnHidden(0, true);

    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    layout->addWidget(view);

    // Кнопки управления
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить");
    QPushButton *deleteButton = new QPushButton("Удалить");
    QPushButton *saveButton = new QPushButton("Сохранить");

    connect(addButton, &QPushButton::clicked, [model]() {
        model->insertRow(model->rowCount());
    });

    connect(deleteButton, &QPushButton::clicked, [model, view]() {
        model->removeRow(view->currentIndex().row());
    });

    connect(saveButton, &QPushButton::clicked, [model]() {
        if (!model->submitAll()) {
            QMessageBox::critical(nullptr, "Ошибка", model->lastError().text());
        }
    });

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    layout->addLayout(buttonLayout);
}

QString EmployeeForm::hashPassword(const QString &password) {
    QByteArray hash = QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
        );
    return QString(hash.toHex());
}

void EmployeeForm::saveChanges() {
    if (!model->submitAll()) {
        QMessageBox::warning(this, "Ошибка", model->lastError().text());
    }

    // Обновляем данные в таблице person
    for (int row = 0; row < model->rowCount(); ++row) {
        QSqlRecord record = model->record(row);
        int personId = record.value("id_person").toInt();
        QString name = record.value("name").toString();
        QString secondName = record.value("second_name").toString();
        QString middleName = record.value("middle_name").toString();

        updatePersonRecord(personId, name, secondName, middleName);
    }

    // Обновляем модель
    model->select();
    QMessageBox::information(this, "Успеч", "Данные обнавлены");
}
