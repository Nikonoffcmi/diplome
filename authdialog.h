#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include <QTranslator>
#include <QSqlDatabase>

namespace Ui {
class AuthDialog;
}

class AuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthDialog(QWidget *parent = nullptr, bool *admin = nullptr, int *userId = nullptr);
    ~AuthDialog();

private slots:
    void on_loginButton_clicked();

    void on_languageCombo_currentIndexChanged(int index);

private:
    Ui::AuthDialog *ui;
    QTranslator translator;
    QSqlDatabase db;
    bool *m_admin;
    int *m_user;

    void loadTranslations();
    void updateUI();
    void checkCapsLock();
};

#endif // AUTHDIALOG_H
