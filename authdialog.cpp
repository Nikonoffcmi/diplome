#include "authdialog.h"
#include "ui_authdialog.h"

#include <QMessageBox>
#include <QKeyEvent>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

AuthDialog::AuthDialog(QWidget *parent, bool *admin, int *userId)
    : QDialog(parent)
    , ui(new Ui::AuthDialog)
{
    ui->setupUi(this);
    installEventFilter(this);

    loadTranslations();

    updateUI();
    checkCapsLock();
    m_admin = admin;
    m_user = userId;
}

AuthDialog::~AuthDialog()
{
    delete ui;
}


void AuthDialog::on_loginButton_clicked()
{
    QString login = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Пожалуйста, заполните все поля"));
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM employee WHERE login = :login AND password = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", password);

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Ошибка"), tr("Ошибка базы данных: ") + query.lastError().text());
        return;
    }

    if (query.next()) {
        *m_admin = query.value("admin").toBool();
        *m_user = query.value("id_employee").toInt();
        accept();
    } else {
        QMessageBox::warning(this, tr("Ошибка"), tr("Неверный логин или пароль"));
    }

}


void AuthDialog::on_languageCombo_currentIndexChanged(int index)
{
    QString lang = (index == 0) ? "ru_RU" : "en_US";
    translator.load(":/diplome_" + lang + ".qm");
    updateUI();
}

void AuthDialog::loadTranslations()
{
    ui->languageCombo->addItem("Русский");
    ui->languageCombo->addItem("English");
    translator.load(":/diplome_ru_RU.qm");
    qApp->installTranslator(&translator);
}

void AuthDialog::updateUI()
{
    ui->retranslateUi(this);
    checkCapsLock();
}

void AuthDialog::checkCapsLock()
{
    bool capsLockOn = (QGuiApplication::keyboardModifiers() & Qt::Key_CapsLock);
    ui->capsLockLabel->setVisible(capsLockOn);
    ui->capsLockLabel->setText(capsLockOn ? tr("Caps Lock is ON") : "");
}
