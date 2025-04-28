#include "reportcreate.h"
#include "ui_reportcreate.h"

// #include <quazip.h>
// #include <quazipfile.h>


ReportCreate::ReportCreate(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportCreate)
{
    ui->setupUi(this);

    connect(ui->generateButton, &QPushButton::clicked, [this]() {
        int productId = ui->productCombo->currentData().toInt();
        ActData data = fetchActData(productId);
        // generatePdf(data);
        // Или generateDocx(data);
    });
}

ReportCreate::~ReportCreate()
{
    delete ui;
}

ActData ReportCreate::fetchActData(int productId) {
    ActData data;
    QSqlQuery query;
    query.prepare(R"(
        SELECT ... /* Ваш SQL-запрос из предыдущего ответа */
        WHERE p.id_product = ?
    )");
    query.addBindValue(productId);

    if (query.exec()) {
        while (query.next()) {
            // Парсим результаты запроса
            data.productSerial = query.value("product_serial").toString();
            // ... заполняем все поля
        }
    }
    return data;
}

// void generatePdf(const ActData& data) {
//     QTextDocument doc;

//     QString html = R"(
//         <h1>Акт приемки защитного покрытия</h1>
//         <p>Изделие: %1</p>
//         <p>Партия: %2</p>
//         <table border='1'>
//             <tr><th>Точка измерения</th><th>Значение</th><th>Норма</th></tr>
//             %3
//         </table>
//         <p>Результат: %4</p>
//     )";

//     QString rows;
//     for (int i = 0; i < data.measurements.size(); ++i) {
//         rows += QString("<tr><td>%1</td><td>%2</td><td>%3-%4</td></tr>")
//         .arg(data.measurementPoints[i])
//             .arg(data.measurements[i])
//             .arg(data.lowerLimit).arg(data.upperLimit);
//     }

//     html = html.arg(data.productSerial)
//                .arg(data.batchNumber)
//                .arg(rows)
//                .arg(data.qualityStatus ? "Соответствует" : "Не соответствует");

//     doc.setHtml(html);

//     QPrinter printer(QPrinter::PrinterResolution);
//     printer.setOutputFormat(QPrinter::PdfFormat);
//     printer.setOutputFileName("акт.pdf");
//     doc.print(&printer);
// }

// void generateDocx(const ActData& data) {
//     QuaZip zip("template.docx");
//     zip.open(QuaZip::mdUnzip);

//     // Извлекаем и модифицируем document.xml
//     QuaZipFile file(&zip);
//     zip.setCurrentFile("word/document.xml");
//     file.open(QIODevice::ReadOnly);
//     QString xml = file.readAll();
//     file.close();

//     // Замена меток
//     xml.replace("${PRODUCT_SERIAL}", data.productSerial);
//     // ... остальные замены

//     // Сохраняем изменения
//     zip.close();
//     zip.open(QuaZip::mdCreate | QuaZip::mdAppend);
//     // ... запись модифицированных файлов обратно в ZIP
// }
