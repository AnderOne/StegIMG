#-------------------------------------------------
#
# Project created by QtCreator 2019-03-24T13:47:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

TARGET = StegIMG

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -fpermissive

INCLUDEPATH += inc

HEADERS += \
    inc/stegmap.hpp \
    inc/stegarch.hpp \
    inc/binstream.hpp \
    inc/rlestream.hpp \
    inc/lzwstream.hpp \
    inc/imageview.hpp \
    inc/compressordialog.hpp \
    inc/mainwindow.hpp

SOURCES += \
    src/stegmap.cpp \
    src/stegarch.cpp \
    src/binstream.cpp \
    src/rlestream.cpp \
    src/lzwstream.cpp \
    src/imageview.cpp \
    src/compressordialog.cpp \
    src/mainwindow.cpp \
    src/main.cpp

FORMS += \
    gui/compressordialog.ui \
    gui/mainwindow.ui
