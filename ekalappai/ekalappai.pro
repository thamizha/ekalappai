HEADERS = window.h \
    about.h
SOURCES = main.cpp \
    window.cpp \
    about.cpp
RESOURCES = ekalappai.qrc
QT += xml \
    svg
OTHER_FILES += ekalappai.rc
RC_FILE = ekalappai.rc

FORMS += \
    about.ui
