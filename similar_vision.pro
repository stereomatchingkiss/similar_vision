#-------------------------------------------------
#
# Project created by QtCreator 2016-06-27T15:15:00
#
#-------------------------------------------------

QT       += core gui concurrent
#CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = similar_vision
TEMPLATE = app

INCLUDEPATH += ui
INCLUDEPATH += ../

include(../pri/boost.pri)
include(../pri/cv_dev.pri)


SOURCES += main.cpp\
        mainwindow.cpp \
    ui/paint_custom_words.cpp \
    model/folder_model.cpp \
    core/scan_folder.cpp \
    core/pics_find_img_hash.cpp \
    model/duplicate_img_model.cpp

HEADERS  += mainwindow.hpp \
    ui/paint_custom_words.hpp \
    model/folder_model.hpp \
    core/scan_folder.hpp \
    core/pics_find_img_hash.hpp \
    core/vp_tree.hpp \
    model/duplicate_img_model.hpp

FORMS    += mainwindow.ui

RESOURCES += \
    pics.qrc
