#ifndef MEASUREMENTDEVICEFORM_H
#define MEASUREMENTDEVICEFORM_H

#include "qtableview.h"
#include "qtabwidget.h"
#include <QDialog>
#include <QSqlRelationalTableModel>
#include <QDateEdit>

namespace Ui {
class MeasurementDeviceForm;
}

class MeasurementDeviceForm : public QDialog
{
    Q_OBJECT

public:
    explicit MeasurementDeviceForm(bool isAdmin, QWidget *parent = nullptr);
    ~MeasurementDeviceForm();

private slots:
    void addDevice();
    void saveChanges();
    void deleteRecord();

private:
    Ui::MeasurementDeviceForm *ui;
    void setupUI();
    void setupManufacturerTab();
    void setupMeasurementValueTab();
    void setupCharacteristicsTab();
    void setupDeviceModelTab();
    void setupDeviceTab();

    bool isAdmin;
    QTabWidget *tabs;
    QSqlRelationalTableModel *deviceModel;
    QTableView *deviceView;
    QDateEdit *verificationDateEdit;
    QDateEdit *repairDateEdit;
};

#endif // MEASUREMENTDEVICEFORM_H
