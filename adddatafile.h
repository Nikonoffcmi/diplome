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

namespace Ui {
class AddDataFile;
}

class AddDataFile : public QDialog
{
    Q_OBJECT

public:
    explicit AddDataFile(QWidget *parent = nullptr, QSqlDatabase db = QSqlDatabase::database());
    ~AddDataFile();

    void show()
    {
        if (m_operationSuccessful) {
            QDialog::show();
        }
    }

    void loadDataFromExcel(const QString &filePath);

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
    void validateInspector();

    void onInstrumentSerialSelected(const QString &text);
    void onProductSerialSelected(const QString &text);

private:
    Ui::AddDataFile *ui;
    bool m_operationSuccessful = true;
    QStandardItemModel *excelModel;

    DataManager m_dataManager;

    void setupCompleters();
    void connectSignals();
};

#endif // ADDDATAFILE_H
