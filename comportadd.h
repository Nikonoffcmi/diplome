#ifndef COMPORTADD_H
#define COMPORTADD_H

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QCompleter>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QItemSelectionModel>

#include "datamanager.h"

namespace Ui {
class COMportAdd;
}

struct CommandStruct
{
    quint8 header[2];
    quint8 command;
    quint8 data;
    quint8 footer[2];
};

class COMportAdd : public QDialog
{
    Q_OBJECT

public:
    explicit COMportAdd(QWidget *parent = nullptr, int userId = 0);
    ~COMportAdd();


    void readData();

    quint64 sendCommand();

    qint64 write(QByteArray data);

private slots:
    void on_btnOpenPort_clicked();

    void on_btnStart_clicked();

    void on_btnStop_clicked();

    void on_btnAddData_clicked();

    void on_cmbPorts_currentIndexChanged(int index);

    void on_pushButton_clicked();

private:
    Ui::COMportAdd *ui;
    QSerialPort *_serialPort;
    CommandStruct _struct;
    DataManager m_dataManager;
    QStandardItemModel *dataModel;
    int m_userId;

    void setupCompleters();
    void connectSignals();
    void updateInstrumentSerials();
    void updateProductSerials();
    void updatePoints();
    void validateInstrument();
    void validateInstrumentSerial();
    void validateProduct();
    void validateProductSerial();
    void validateInspector();
    bool eventFilter(QObject *obj, QEvent *event);
    QString searchDataDB(const QString &idName, const QString &data, const QString &dataTable, const QString &dataWhere);

    void onInstrumentSerialSelected(const QString &text);
    void onProductSerialSelected(const QString &text);

    void loadPorts();
    void processJson(const QByteArray &jsonData);
    bool validateMeasurement(double value, int placeId);
    bool saveToDatabase(double value, int placeId, int serial);
    int getDeviceId(const QString &serial);
    int getPlaceId(const QString &plase);
    int checkQuantity(const int device_id, const QString &quantity);
};

#endif // COMPORTADD_H
