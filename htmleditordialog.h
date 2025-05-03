#ifndef HTMLEDITORDIALOG_H
#define HTMLEDITORDIALOG_H

#include "qtextedit.h"
#include <QDialog>

namespace Ui {
class HtmlEditorDialog;
}

class HtmlEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HtmlEditorDialog(QWidget *parent = nullptr);
    ~HtmlEditorDialog();

    void setHtml(const QString &html) {
        editor->setPlainText(html);
    }

    QString getHtml() const {
        return editor->toPlainText();
    }
private:
    Ui::HtmlEditorDialog *ui;
    QTextEdit *editor;
    QPushButton *btnSave;
    QPushButton *btnCancel;
};

#endif // HTMLEDITORDIALOG_H
