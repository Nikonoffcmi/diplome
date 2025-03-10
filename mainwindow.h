#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <QCheckBox>
#include <QSortFilterProxyModel>
#include <QString>
#include <QFile>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ScanningFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ScanningFilterModel(QObject *parent = nullptr);

    // Метод для установки фильтра по столбцу
    void set_filter(int column, const QString &filter);

protected:
    // Переопределяем метод фильтрации
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QMap<int, QString> m_filters;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QWidget* create_filter_widget_by_scanning_column(const int &column) const;

private slots:
    void on_action_triggered();

    void on_pushButton_clicked();

    void on_action_4_triggered();

    void on_action_5_triggered();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlQueryModel* model;
    ScanningFilterModel* filModel;
};



#endif // MAINWINDOW_H
