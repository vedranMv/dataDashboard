android|ios|winrt {
    error( "This example is not supported for android, ios, or winrt." )
}

INCLUDEPATH += ./include

LIBS += -L$$OUT_PWD/../../../lib

TEMPLATE = app

QT += datavisualization

contains(TARGET, qml.*) {
    QT += qml quick
}

target.path = ./build/$$TARGET
INSTALLS += target


SOURCES += main.cpp scatter/scatterdatamodifier.cpp \
        helperObjects/channel/channel.cpp \
        mainwindow.cpp \
        orientation_3d/orientationwidget.cpp \
        orientation_3d/geometryengine.cpp \
        serialadapter/serialadapter.cpp


HEADERS += scatter/scatterdatamodifier.h \
    helperObjects/channel/channel.h \
    mainwindow.h \
           scatter/ScatterDataPlot.h \
    orientation_3d/orientationwidget.h \
    orientation_3d/geometryengine.h \
    serialAdapter/serialAdapter.h

RESOURCES += \
    orientation_3d/resources/shaders.qrc \
    orientation_3d/resources/textures.qrc \
    config.ini

QT += core gui widgets serialport
requires(qtConfig(combobox))
requires(qtConfig(fontcombobox))

FORMS += \
    mainwindow.ui

