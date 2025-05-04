QT       += core gui axcontainer serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql charts

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adddatafile.cpp \
    authdialog.cpp \
    comportadd.cpp \
    datamanager.cpp \
    employeeform.cpp \
    exportdialog.cpp \
    htmleditordialog.cpp \
    main.cpp \
    mainwindow.cpp \
    measurementdeviceform.cpp \
    productform.cpp \
    productmanagementform.cpp \
    reportcreate.cpp \
    settingsdialog.cpp

HEADERS += \
    adddatafile.h \
    authdialog.h \
    comportadd.h \
    datamanager.h \
    employeeform.h \
    exportdialog.h \
    htmleditordialog.h \
    mainwindow.h \
    measurementdeviceform.h \
    productform.h \
    productmanagementform.h \
    reportcreate.h \
    settingsdialog.h

FORMS += \
    adddatafile.ui \
    authdialog.ui \
    comportadd.ui \
    employeeform.ui \
    exportdialog.ui \
    htmleditordialog.ui \
    mainwindow.ui \
    measurementdeviceform.ui \
    productform.ui \
    productmanagementform.ui \
    reportcreate.ui \
    settingsdialog.ui

TRANSLATIONS += \
    diplome_en_US.ts \
    diplome_ru_RU.ts
CONFIG += lrelease
# CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# QXlsx code for Application Qt project
QXLSX_PARENTPATH=./         # current QXlsx path is . (. means curret directory)
QXLSX_HEADERPATH=./header/  # current QXlsx header path is ./header/
QXLSX_SOURCEPATH=./source/  # current QXlsx source path is ./source/
include(./QXlsx.pri)
