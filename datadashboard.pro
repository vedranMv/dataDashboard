TEMPLATE = app

QT += datavisualization core gui widgets serialport printsupport


SOURCES += main.cpp scatter/scatterwindow.cpp \
        helperObjects/channel/channel.cpp \
        helperObjects/dataMultiplexer/datamultiplexer.cpp \
        helperObjects/graphHeaderWidget/graphheaderwidget.cpp \
        helperObjects/mathComponent/mathchannelcomponent.cpp \
        line/lineplot.cpp \
        line/qcustomplot.cpp \
        mainwindow.cpp \
        orientation_3d/orientationwidget.cpp \
        orientation_3d/geometryengine.cpp \
        orientation_3d/orientationwindow.cpp \
        serialAdapter/serialadapter.cpp


HEADERS += scatter/scatterwindow.h \
    helperObjects/channel/channel.h \
    helperObjects/dataMultiplexer/datamultiplexer.h \
    helperObjects/dataMultiplexer/graphclient.h \
    helperObjects/dataMultiplexer/mathchannel.h \
    helperObjects/graphHeaderWidget/graphheaderwidget.h \
    helperObjects/mathComponent/mathchannelcomponent.h \
    line/lineplot.h \
    line/qcustomplot.h \
    mainwindow.h \
    orientation_3d/orientationwindow.h \
    orientation_3d/orientationwidget.h \
    orientation_3d/geometryengine.h \
    serialAdapter/serialadapter.h

RESOURCES += \
    orientation_3d/resources/shaders.qrc \
    orientation_3d/resources/textures.qrc \
    config.ini

requires(qtConfig(combobox))
requires(qtConfig(fontcombobox))

FORMS += \
    mainwindow.ui

