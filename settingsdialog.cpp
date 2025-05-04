#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QFontDialog>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    loadLanguages();
    loadSettings();
}


void SettingsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void SettingsDialog::on_fontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QApplication::font(), this);
    if (ok) {
        updateFont(font);
    }
}

void SettingsDialog::on_languageCombo_currentIndexChanged(int index)
{
    QString langCode = ui->languageCombo->itemData(index).toString();
    updateLanguage(langCode);
}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    QFont font = settings.value("Font", QApplication::font()).value<QFont>();
    QString lang = settings.value("Language", "ru_RU").toString();

    updateFont(font);
    updateLanguage(lang);
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("Font", QApplication::font());
    settings.setValue("Language", ui->languageCombo->currentData().toString());
}

void SettingsDialog::updateFont(const QFont &font)
{
    QApplication::setFont(font);
}

void SettingsDialog::updateLanguage(const QString &language)
{

    QString lang = (language == "ru_RU") ? "ru_RU" : "en_US";
    qDebug() << language;
    appTranslator.load("diplome_" + lang + ".qm");
    qApp->installTranslator(&appTranslator);
    ui->retranslateUi(this);
}

void SettingsDialog::loadLanguages()
{
    ui->languageCombo->addItem(tr("Russian"), "ru_RU");
    ui->languageCombo->addItem(tr("English"), "en_US");
    // Добавьте другие языки
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
