#ifndef PRODUCTMANAGEMENTFORM_H
#define PRODUCTMANAGEMENTFORM_H

#include "qtableview.h"
#include "qtabwidget.h"
#include <QDialog>
#include <QSqlRelationalTableModel>


namespace Ui {
class ProductManagementForm;
}

class ProductManagementForm : public QDialog
{
    Q_OBJECT

public:
    ProductManagementForm(bool isAdmin, QWidget *parent = nullptr);
    ~ProductManagementForm();

private slots:
    void addRecord();
    void deleteRecord();
    void saveChanges();

private:
    Ui::ProductManagementForm *ui;

    bool isAdmin;
    QMap<QString, QSqlTableModel*> modelsMap;
    QTabWidget *tabs;
    QSqlRelationalTableModel *currentModel;
    QTableView *currentView;

    void setupProductTypeTab();
    void setupBatchTab();
    void setupMeasuringPointTab();
    void setupPlaceMeasurementTab();

};

#endif // PRODUCTMANAGEMENTFORM_H
