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

QVector<MeasurementData> ReportCreate::getMeasurements(int productId) {
    QVector<MeasurementData> data;
    QSqlQuery query;
    query.prepare(
        "SELECT m.value_measurement, mp.point, md.device_serial, m.quality_protective_layer, "
        "p.name, p.second_name "
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
    // Получение данных
    QSqlQuery productQuery;
    productQuery.prepare("SELECT product_serial, id_batch FROM product WHERE id_product = ?");
    productQuery.addBindValue(productId);
    productQuery.exec();
    productQuery.first();

    QString serial = productQuery.value(0).toString();

    auto measurements = getMeasurements(productId);

    // Генерация HTML
    QString html;
    html += "<html><head><style>"
            "body { font-family: Arial; }"
            "h1 { color: #333; }"
            "table { border-collapse: collapse; width: 100%; }"
            "th, td { border: 1px solid #ddd; padding: 8px; }"
            "th { background-color: #f2f2f2; }"
            ".signature { margin-top: 50px; }"
            "</style></head><body>";

    html += QString("<h1>Акт о приемке защитного покрытия</h1>"
                    "<p><strong>Изделие:</strong> %1</p>"
                    "<h3>Результаты измерений:</h3>"
                    "<table>"
                    "<tr><th>Точка</th><th>Значение</th><th>Соответствие</th><th>Прибор</th><th>Инспектор</th></tr>")
                .arg(serial);

    for (const auto &m : measurements) {
        html += QString("<tr>"
                        "<td>%1</td>"
                        "<td>%2</td>"
                        "<td>%3</td>"
                        "<td>%4</td>"
                        "<td>%5</td>"
                        "</tr>")
                    .arg(m.point)
                    .arg(m.value)
                    .arg(m.quality ? "Да" : "Нет")
                    .arg(m.device)
                    .arg(m.inspector);
    }

    html += "</table>"
            "<div class='signature'>"
            "<p>Проверил: _________________________</p>"
            "<p>Дата: " + QDate::currentDate().toString("dd.MM.yyyy") + "</p>"
                                                            "</div></body></html>";

    html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
           + html.mid(html.indexOf("<style>"));
    currentHtml = html;
    htmlPreview->setHtml(html);
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
