HEADERS = window.h \
    about.h \
    ekeventfilter.h

SOURCES = main.cpp \
    window.cpp \
    about.cpp \
    ekeventfilter.cpp
RESOURCES = ekalappai.qrc
QT += xml \
    svg \
    widgets \
    gui-private
OTHER_FILES += ekalappai.rc
RC_FILE = ekalappai.rc

FORMS += \
    about.ui
