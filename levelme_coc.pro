TEMPLATE = app

QT += qml quick widgets

SOURCES += \
    src/botprogram.cpp \
    src/cocbattlefield.cpp \
    src/main.cpp

RESOURCES += \
    qml/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/botprogram.h \
    src/cocbattlefield.h

LIBS += \
    -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgproc

DISTFILES += \
    README.md \
    qml/MainForm.ui.qml \
    qml/DamageArea.qml \
    qml/main.qml
