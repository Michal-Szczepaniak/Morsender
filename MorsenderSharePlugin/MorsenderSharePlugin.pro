TEMPLATE = lib

TARGET = $$qtLibraryTarget(MorsenderSharePlugin)
CONFIG += plugin
DEPENDPATH += .

CONFIG += link_pkgconfig
PKGCONFIG += nemotransferengine-qt5

CONFIG += c++11

HEADERS += \
    $$PWD/MorsenderPluginInfo.h \
    $$PWD/MorsenderMediaTransfer.h \
    $$PWD/MorsenderSharePlugin.h

SOURCES += \
    $$PWD/MorsenderPluginInfo.cpp \
    $$PWD/MorsenderMediaTransfer.cpp \
    $$PWD/MorsenderSharePlugin.cpp

target.path = /usr/lib/nemo-transferengine/plugins
INSTALLS += target

