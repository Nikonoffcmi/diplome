#include "reportcreate.h"
#include "htmleditordialog.h"
#include "ui_reportcreate.h"

#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

ReportCreate::ReportCreate(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportCreate)
{
    ui->setupUi(this);
    loadProducts();

    connect(ui->printBtn, &QPushButton::clicked, this, &ReportCreate::printPdf);
    htmlPreview = ui->textEdit_2;
    htmlPreview->setAcceptRichText(true);
    htmlPreview->setReadOnly(false);

    connect(ui->editHtmlBtn, &QPushButton::clicked, this, &ReportCreate::openHtmlEditor);


    connect(ui->refreshPreviewBtn, &QPushButton::clicked, [this]() {
        int productId = ui->productCombo->currentData().toInt();
        generateHtmlPreview(productId);
    });

    connect(ui->savePdfBtn, &QPushButton::clicked, this, &ReportCreate::saveEditedPdf);
    ui->editHtmlBtn->setVisible(0);
    ui->buttonBox->removeButton(ui->buttonBox->buttons()[1]);
}

ReportCreate::~ReportCreate()
{
    delete ui;
}

void ReportCreate::loadProducts() {
    QSqlQuery query("SELECT id_product, product_serial FROM product");
    while (query.next()) {
        ui->productCombo->addItem(query.value(1).toString(), query.value(0));
    }
}

QString ReportCreate::loadTemplate() {
    QFile file("template.html");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Template file not found!";
        return QString();
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    return in.readAll();
}

QString ReportCreate::generateReport(int productId) {
    QString templateHtml = loadTemplate();
    if (templateHtml.isEmpty()) return QString();

    return replacePlaceholders(templateHtml, productId);
}

QString ReportCreate::replacePlaceholders(const QString &templateHtml, int productId) {
    QSqlQuery productQuery;
    productQuery.prepare("SELECT P.product_serial, PT.name, M.datetime::timestamp::date FROM public.product P \
                    JOIN product_type PT \
                    ON PT.id_product_type = P.id_product_type \
                    JOIN measurement M \
                    ON M.id_product = P.id_product \
                    WHERE P.id_product = ?");
    productQuery.addBindValue(productId);
    productQuery.exec();
    productQuery.first();

    QString serial = productQuery.value(0).toString();
    QString name = productQuery.value(1).toString();
    QString date = productQuery.value(2).toString();
    QDate realDate = QDate::fromString(date,"yyyy-dd-MM");

    auto measurements = getMeasurements(productId);

    QString resultHtml = templateHtml;

    resultHtml.replace("{{DataNow}}", QDate::currentDate().toString("dd.MM.yyyy"))
        .replace("{{namw}}", name)
        .replace("{{serial}}", serial)
        .replace("{{realDate}}", realDate.toString())
        .replace("{{productQuery}}", QString::number(productQuery.size()));

    QString insp;
    QString rowsHtml;
    for (const auto &m : measurements) {
        insp = m.inspector;
        rowsHtml += QString("<tr>"
                        "<td>Толщина защитного слоя в точке %1</td>"
                        "<td>ГОСТ Р 58973— 2020</td>"
                        "<td>%2</td>"
                        "<td>%3-%4</td>"
                        "<td>%5</td>"
                        "</tr>")
                    .arg(m.point)
                    .arg(m.device)
                    .arg(m.min)
                    .arg(m.max)
                    .arg(m.value);
    }

    resultHtml.replace("{{measurements_rows}}", rowsHtml)
        .replace("{{insp}}", insp);

    return resultHtml;
}

QVector<MeasurementData> ReportCreate::getMeasurements(int productId) {
    QVector<MeasurementData> data;
    QSqlQuery query;
    query.prepare(
        "SELECT m.value_measurement, mp.point, md.device_serial, m.quality_protective_layer, "
        "p.name, p.second_name, pl.upper_limit_thickness, pl.lower_limit_thickness "
        "FROM measurement m "
        "JOIN place_measurement pl ON m.id_place_measurement = pl.id_place_measurement "
        "JOIN measuring_point mp ON pl.id_measurement_point = mp.id_measuring_point "
        "JOIN measuring_device md ON m.id_device = md.id_measuring_device "
        "JOIN employee e ON m.id_employee = e.id_employee "
        "JOIN person p ON e.id_person = p.id_person "
        "WHERE m.id_product = ?"
        );
    query.addBindValue(productId);
    query.exec();

    while (query.next()) {
        MeasurementData d;
        d.value = query.value(0).toDouble();
        d.point = query.value(1).toString();
        d.device = query.value(2).toString();
        d.quality = query.value(3).toBool();
        d.inspector = query.value(4).toString() + " " + query.value(5).toString();
        d.max = query.value(6).toDouble();
        d.min = query.value(7).toDouble();
        data.append(d);
    }
    return data;
}

void ReportCreate::printPdf() {
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter;
        painter.begin(&printer);
        // ... аналогичная логика рисования ...
        painter.end();
    }
}

void ReportCreate::generateHtmlPreview(int productId) {
    currentHtml = generateReport(productId);
    htmlPreview->setHtml(currentHtml);
}

void ReportCreate::saveEditedPdf() {
    QString editedHtml = htmlPreview->toHtml();

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);

    // 3. Зададим явные параметры страницы
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    // 4. Диалог выбора файла
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить PDF",
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "PDF Files (*.pdf)");

    if (fileName.isEmpty()) return;

    printer.setOutputFileName(fileName);

    QTextDocument doc;

    // 5. Установим правильный размер документа
    doc.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());
    doc.setHtml(editedHtml);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось начать запись в PDF: ") + printer.printerName());
        return;
    }

    // 6. Рендеринг с учетом масштаба
    doc.drawContents(&painter, QRectF(0, 0,
                                      printer.pageRect(QPrinter::DevicePixel).width(),
                                      printer.pageRect(QPrinter::DevicePixel).height()));

    painter.end();

    // 7. Проверка результата
    if (QFile::exists(fileName)) {
        QMessageBox::information(this, tr("Успешно"), tr("Файл сохранен: ") + fileName);
    } else {
        QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось сохранить файл"));
    }
}

void ReportCreate::openHtmlEditor() {
    HtmlEditorDialog dlg(this);
    dlg.setHtml(currentHtml);

    if (dlg.exec() == QDialog::Accepted) {
        currentHtml = dlg.getHtml();
        htmlPreview->setHtml(currentHtml);
        QMessageBox::information(this, tr("Успешно"), tr("HTML обновлен! Не забудьте сохранить PDF."));
    }
}
