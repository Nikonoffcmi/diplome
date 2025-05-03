#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QVector>
#include <QCheckBox>
#include <QComboBox>
#include <QSqlQuery>

struct ExportField {
    QString displayName;
    QString columnName;
    QCheckBox* checkbox;
};

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

private slots:
    void exportData();
    void selectAll();
    void deselectAll();

private:
    Ui::ExportDialog *ui;
    QVector<ExportField> fields;
    QComboBox *formatCombo;
    void setupUI();
    void setupFields();
    QString getSaveFileName(const QString &suffix);
};

#endif // EXPORTDIALOG_H
