#include "htmleditordialog.h"
#include "qboxlayout.h"
#include "qpushbutton.h"
#include "ui_htmleditordialog.h"

HtmlEditorDialog::HtmlEditorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HtmlEditorDialog)
{
    ui->setupUi(this);
    editor = new QTextEdit(this);
    editor->setFont(QFont("Courier New", 10));
    editor->setLineWrapMode(QTextEdit::NoWrap);

    btnSave = new QPushButton(tr("Сохранить"), this);
    btnCancel = new QPushButton(tr("Отмена"), this);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(btnSave);
    buttonLayout->addWidget(btnCancel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(editor);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Редактор HTML"));
    resize(800, 600);

    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

HtmlEditorDialog::~HtmlEditorDialog()
{
    delete ui;
}
