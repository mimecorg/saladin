HEADERS += xmlui/builder.h \
           xmlui/client.h \
           xmlui/gradientwidget.h \
           xmlui/node_p.h \
           xmlui/toolstrip.h \
           xmlui/toolstrip_p.h

SOURCES += xmlui/builder.cpp \
           xmlui/client.cpp \
           xmlui/gradientwidget.cpp \
           xmlui/toolstrip.cpp

win32 {
    HEADERS += xmlui/windowsstyle.h
    SOURCES += xmlui/windowsstyle.cpp
}
