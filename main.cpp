#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // QFile styleSheetFile("./Integrid.qss");
    // styleSheetFile.open(QFile::ReadOnly);
    // QString styleSheet = QLatin1String(styleSheetFile.readAll());
    // a.setStyleSheet(styleSheet);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "diplome_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
