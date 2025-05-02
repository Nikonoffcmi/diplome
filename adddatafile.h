#ifndef ADDDATAFILE_H
#define ADDDATAFILE_H

#include "datamanager.h"
#include <QDialog>
#include <QFileDialog>
#include <QFile>
#include <QAxObject>
#include <QTableView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDebug>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QCompleter>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QMessageBox>
#include <QItemSelectionModel>

namespace Ui {
class AddDataFile;
}

class AddDataFile : public QDialog
{
    Q_OBJECT

public:
    explicit AddDataFile(QWidget *parent = nullptr, int userId = 0);
    ~AddDataFile();

    void show()
    {
        if (m_operationSuccessful) {
            QDialog::show();
        }
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_CloseBtn_clicked();

    void updateInstrumentSerials();
    void updateProductSerials();

    void validateInstrument();
    void validateInstrumentSerial();
    void validateProduct();
    void validateProductSerial();

    void onInstrumentSerialSelected(const QString &text);
    void onProductSerialSelected(const QString &text);

    void on_AccBtn_clicked();

private:
    Ui::AddDataFile *ui;
    QStandardItemModel *tableModel;
    bool m_operationSuccessful = true;
    QStandardItemModel *dataModel;
    int m_userId;

    void loadTextFile(const QString &fileName);
    void loadExcelFile(const QString &fileName);

    DataManager m_dataManager;

    void setupCompleters();
    void connectSignals();

    bool insertIntoDatabase(double value, QString Date, QString product, int placeId, QString serial);
    int getPlaceId(const QString &plase);
    bool validateMeasurement(double value, int placeId);
    QString searchDataDB(const QString &idName, const QString &data, const QString &dataTable, const QString &dataWhere);
};

#endif // ADDDATAFILE_H
