#-------------------------------------------------
#
# Project created by QtCreator 2014-02-10T20:44:20
#
#-------------------------------------------------

QT       -= gui

TARGET = ekhook
TEMPLATE = lib

DEFINES += EKHOOK_LIBRARY

SOURCES += ekhook.cpp \
    dllmain.cpp \
    stdafx.cpp

HEADERS += \
    stdafx.h \
    targetver.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
