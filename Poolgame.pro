#-------------------------------------------------
#
# Project created by QtCreator 2018-04-23T17:08:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Poolgame
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


SOURCES += \
        main.cpp \
        dialog.cpp \
    stageonefactory.cpp \
    ball.cpp \
    table.cpp \
    game.cpp \
    gamebuilder.cpp \
    stagetwofactory.cpp \
    pocket.cpp \
    stagetwobuilder.cpp \
    balldecorator.cpp \
    strategy.cpp \
    visiter.cpp

HEADERS += \
        dialog.h \
    abstractstagefactory.h \
    stageonefactory.h \
    ball.h \
    table.h \
    game.h \
    gamebuilder.h \
    utils.h \
    stagetwofactory.h \
    pocket.h \
    stagetwobuilder.h \
    balldecorator.h \
    mouseeventable.h \
    memento.h \
    originator.h \
    strategy.h \
    visiter.h

FORMS += \
        dialog.ui

DISTFILES += \
    config.json \
    config.json

copyconfig.depends = ../Poolgame/config.json
copyconfig.commands = \
        $(COPY_FILE) \"$$shell_path($$PWD\\config.json)\" .

first.depends = all copyconfig
QMAKE_EXTRA_TARGETS += first copyconfig
