#ifndef EMPLOYEEFORM_H
#define EMPLOYEEFORM_H

#include "qsqlrelationaltablemodel.h"
#include "qsqltablemodel.h"
#include "qtableview.h"
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlError>

#include <QSqlQueryModel>

namespace Ui {
class EmployeeForm;
}

class EmployeeForm : public QDialog
{
    Q_OBJECT

public:
    explicit EmployeeForm(bool isAdmin, int userId, QWidget *parent = nullptr);
    ~EmployeeForm();

private slots:
    void addEmployee();
    void deleteEmployee();
    void saveChanges();

private:
    Ui::EmployeeForm *ui;
    bool isAdmin;
    int userId;
    QSqlRelationalTableModel* model;
    QTableView *view;

    void setupEmployeeTab(QWidget *tab);
    void setupDivisionTab(QWidget *tab);
    void updatePersonRecord(int personId, const QString &name,
                            const QString &secondName, const QString &middleName);
    void setupPostTab(QWidget *tab);
    void setupUserForm();
    QString hashPassword(const QString &password);

};

#endif // EMPLOYEEFORM_H
