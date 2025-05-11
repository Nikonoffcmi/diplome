#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTranslator>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
protected:
    void changeEvent(QEvent *event) override;

private slots:
    void on_fontButton_clicked();
    void on_languageCombo_currentIndexChanged(int index);

private:
    Ui::SettingsDialog *ui;
    QTranslator *appTranslator;
    void loadSettings();
    void saveSettings();
    void updateFont(const QFont &font);
    void updateLanguage(const QString &language);
    void loadLanguages();
};

#endif // SETTINGSDIALOG_H
