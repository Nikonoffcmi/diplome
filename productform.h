#ifndef PRODUCTFORM_H
#define PRODUCTFORM_H

#include "qboxlayout.h"
#include "qtableview.h"
#include <QDialog>
#include <QSqlRelationalTableModel>

namespace Ui {
class ProductForm;
}

class ProductForm : public QDialog
{
    Q_OBJECT

public:
    explicit ProductForm(bool isAdmin,QWidget *parent = nullptr);
    ~ProductForm();

private slots:
    void addProduct();
    void deleteProduct();
    void saveChanges();

private:
    Ui::ProductForm *ui;
    void setupUI();
    void setupModel();
    void setupConnections();
    void setupSearch(QVBoxLayout *mainLayout);

    bool isAdmin;
    QSqlRelationalTableModel *model;
    QTableView *view;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *saveButton;
};

#endif // PRODUCTFORM_H
