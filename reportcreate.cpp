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
    // Получение данных
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

    // Генерация HTML
    QString html;
    html = "<!DOCTYPE html> \
           <html lang='ru'> \
          <head> \
                  <meta charset='UTF-8'>\
                  <title>Протокол испытаний №5097/5097-АС-03</title> \
          <style>\
          body { font-family: Arial, sans-serif; margin: 20px; } \
    .header { text-align: center; margin-bottom: 20px; }\
    .section { margin-bottom: 25px; }\
    table { width: 100%; border-collapse: collapse; margin: 15px 0; }\
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\
    th { background-color: #f2f2f2; }\
    .signature { margin-top: 30px; }\
    .bold { font-weight: bold; }\
    .underline { text-decoration: underline; }\
    </style>\
        </head>\
        <body>\
        <div class='header'>\
        <h3>ФЕДЕРАЛЬНОЕ АГЕНТСТВО ПО ТЕХНИЧЕСКОМУ РЕГУЛИРОВАНИЮ И МЕТРОЛОГИИ</h3>\
        <address>\
            Федеральное бюджетное учреждение «Государственный региональный центр стандартизации, метрологии и испытаний в Московской области»<br>\
                                                                                                         Орехово-Зуевский филиал ФБУ «ЦСМ Московской области»<br>\
                                                                                                     142608, Московская область, г. Орехово-Зуево, ул. Коминтерна, д. 1<br>\
                                                                  Тел. 412-16-35 Факс 412-16-35\
                                                                      </address>\
                                                                      </div>\
                                                                         \
                                                                          <div class='section'>\
                                                                          <h3>ПРОТОКОЛ № 5097/5097-АС-03 от "+QDate::currentDate().toString("dd.MM.yyyy")+"г.</h3>\
                                                                  </div>\
                                                                  \
                                                                  <div class='section'>\
                                                                  <p><span class='bold'>ИСПЫТУЕМЫЙ ОБРАЗЕЦ:</span> "+name+"</p>\
                                                        </div>\
                                                        \
                                                        <div class='section'>\
                                                        <p><span class='bold'>РЕГИСТРАЦИОННЫЙ НОМЕР:</span> "+serial+"</p>\
                                                                                                         <p><span class='bold'>ЗАКАЗЧИК:</span> ИП Тиханович Александр Эдуардович<br>\
                                                                                                                                             220104 город Минск улица Петра Глебки 112-45, Республика Беларусь</p>\
            <p><span class='bold'>ДАТА ПРОВЕДЕНИЯ ИСПЫТАНИЙ:</span> "+realDate.toString("dd.MM.yyyy")+"г.</p>\
                                                                 <p><span class='bold'>ОБЪЕМ ПРОБЫ:</span> "+QString::number(productQuery.size())+" шт.</p>\
                                                                </div>\
                                                                \
                                                                <table>\
                                                                <thead>\
                                                                <tr>\
                                                                <th>Определяемый показатель</th>\
                                                                <th>Метод испытаний</th>\
                                                                <th>Средства измерений</th>\
                                                                <th>ПДК и нормы</th>\
                                                                <th>Результаты испытаний</th>\
                                                                </tr>\
                                                                </thead>\
                                                                <tbody>";
    QString insp;
    for (const auto &m : measurements) {
        insp = m.inspector;
            html += QString("<tr>"
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
    html+=                          "</tbody>\
                    </table>\
                    \
                    <div class='section'>\
                    <p class='bold>Заключение:</p>\
                                                    <p>Проверенные образцы изделий соответствуют ГОСТ Р 58973— 2020.</p>\
                                                      </div>\
                                                      \
                                                      <div class='signature'>\
                                                      <p>Результаты исследований подтверждаю:</p>\
                                                                                                <p>Ответственный за протокол<br><span class='underline'>"+insp+"</span></p>\
                                                                                                </div>\
                                                                                                                                    </body>\
           </html>";
    // html += "<html><head><style>"
    //         "body { font-family: Arial; }"
    //         "h1 { color: #333; }"
    //         "table { border-collapse: collapse; width: 100%; }"
    //         "th, td { border: 1px solid #ddd; padding: 8px; }"
    //         "th { background-color: #f2f2f2; }"
    //         ".signature { margin-top: 50px; }"
    //         "</style></head><body>";

    // html += QString("<h1>Акт о приемке защитного покрытия</h1>"
    //                 "<p><strong>Изделие:</strong> %1</p>"
    //                 "<h3>Результаты измерений:</h3>"
    //                 "<table>"
    //                 "<tr><th>Точка</th><th>Значение</th><th>Соответствие</th><th>Прибор</th><th>Инспектор</th></tr>")
    //             .arg(serial);

    // for (const auto &m : measurements) {
    //     html += QString("<tr>"
    //                     "<td>%1</td>"
    //                     "<td>%2</td>"
    //                     "<td>%3</td>"
    //                     "<td>%4</td>"
    //                     "<td>%5</td>"
    //                     "</tr>")
    //                 .arg(m.point)
    //                 .arg(m.value)
    //                 .arg(m.quality ? "Да" : "Нет")
    //                 .arg(m.device)
    //                 .arg(m.inspector);
    // }

    // html += "</table>"
    //         "<div class='signature'>"
    //         "<p>Проверил: _________________________</p>"
    //         "<p>Дата: " + QDate::currentDate().toString("dd.MM.yyyy") + "</p>"
    //                                                         "</div></body></html>";

    // html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    //        + html.mid(html.indexOf("<style>"));
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
