HEADERS += $$PWD/builder.h \
           $$PWD/client.h \
           $$PWD/gradientwidget.h \
           $$PWD/node_p.h \
           $$PWD/toolstrip.h \
           $$PWD/toolstrip_p.h

SOURCES += $$PWD/builder.cpp \
           $$PWD/client.cpp \
           $$PWD/gradientwidget.cpp \
           $$PWD/toolstrip.cpp

win32 {
    HEADERS += $$PWD/windowsstyle.h
    SOURCES += $$PWD/windowsstyle.cpp
}
