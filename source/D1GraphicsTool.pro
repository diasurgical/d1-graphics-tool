#-------------------------------------------------
#
# Project created by QtCreator 2018-03-08T22:05:17
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = d1-graphics-tool
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
    mainwindow.cpp \
    d1cel.cpp \
    d1cl2.cpp \
    d1pal.cpp \
    d1trn.cpp \
    celview.cpp \
    palview.cpp \
    d1min.cpp \
    d1til.cpp \
    levelcelview.cpp \
    d1celbase.cpp \
    exportdialog.cpp \
    d1mpq.cpp

HEADERS  += mainwindow.h \
    d1cel.h \
    d1cl2.h \
    d1pal.h \
    d1trn.h \
    celview.h \
    palview.h \
    d1min.h \
    d1til.h \
    levelcelview.h \
    d1celbase.h \
    exportdialog.h \
    d1mpq.h

FORMS += mainwindow.ui \
    celview.ui \
    palview.ui \
    levelcelview.ui \
    exportdialog.ui

RESOURCES += \
    d1files.qrc
