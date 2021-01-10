TEMPLATE = app

QT += datavisualization core gui widgets serialport printsupport network


SOURCES += main.cpp mainwindow.cpp \
        helperObjects/channel/channel.cpp \
        helperObjects/dataMultiplexer/datamultiplexer.cpp \
        helperObjects/graphHeaderWidget/graphheaderwidget.cpp \
        helperObjects/mathComponent/mathchannelcomponent.cpp \
        plotWindows/line/lineplot.cpp \
        plotWindows/line/qcustomplot.cpp \
        plotWindows/scatter/scatterwindow.cpp \
        plotWindows/orientation_3d/orientationwidget.cpp \
        plotWindows/orientation_3d/geometryengine.cpp \
        plotWindows/orientation_3d/orientationwindow.cpp \
        dataSources/networkAdapter/networkadapter.cpp \
        dataSources/serialAdapter/serialadapter.cpp


HEADERS += mainwindow.h \
    dataSources/dataSources.h \
    helperObjects/channel/channel.h \
    helperObjects/dataMultiplexer/datamultiplexer.h \
    helperObjects/dataMultiplexer/graphclient.h \
    helperObjects/dataMultiplexer/mathchannel.h \
    helperObjects/graphHeaderWidget/graphheaderwidget.h \
    helperObjects/mathComponent/mathchannelcomponent.h \
    plotWindows/line/lineplot.h \
    plotWindows/line/qcustomplot.h \
    plotWindows/plotWindows.h \
    plotWindows/scatter/scatterwindow.h \
    plotWindows/orientation_3d/orientationwindow.h \
    plotWindows/orientation_3d/orientationwidget.h \
    plotWindows/orientation_3d/geometryengine.h \
    dataSources/networkAdapter/networkadapter.h \
    dataSources/serialAdapter/serialadapter.h

RESOURCES += \
    res/icon.qrc \
    plotWindows/orientation_3d/resources/shaders.qrc \
    plotWindows/orientation_3d/resources/textures.qrc \
    config.ini

requires(qtConfig(combobox))
requires(qtConfig(fontcombobox))

FORMS += \
    mainwindow.ui

