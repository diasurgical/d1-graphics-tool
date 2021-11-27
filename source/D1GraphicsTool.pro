#-------------------------------------------------
#
# Project created by QtCreator 2018-03-08T22:05:17
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = D1GraphicsTool
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    batchexportdialog.cpp \
    d1palhits.cpp \
    mainwindow.cpp \
    d1cel.cpp \
    d1cl2.cpp \
    d1pal.cpp \
    d1trn.cpp \
    celview.cpp \
    palettewidget.cpp \
    d1min.cpp \
    d1til.cpp \
    levelcelview.cpp \
    d1celbase.cpp \
    exportdialog.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    batchexportdialog.h \
    d1cel.h \
    d1cl2.h \
    d1pal.h \
    d1palhits.h \
    d1trn.h \
    celview.h \
    palettewidget.h \
    d1min.h \
    d1til.h \
    levelcelview.h \
    d1celbase.h \
    exportdialog.h \
    settingsdialog.h

FORMS += mainwindow.ui \
    batchexportdialog.ui \
    celview.ui \
    palettewidget.ui \
    levelcelview.ui \
    exportdialog.ui \
    settingsdialog.ui

RESOURCES += \
    d1files.qrc

RC_ICONS = icon.ico
