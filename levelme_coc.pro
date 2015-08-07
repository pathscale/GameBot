TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp \
    cocbattlefield.cpp \
    botprogram.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    cocbattlefield.h \
    botprogram.h

LIBS += \
    -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgproc
