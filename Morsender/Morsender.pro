# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed
include(vendor/vendor.pri)
include(webp-plugin/libwebp.pri)

# The name of your application
TARGET = Morsender

QT += dbus
CONFIG += link_pkgconfig sailfishapp
#QMAKE_CXXFLAGS += -std=gnu99
#QMAKE_LFLAGS += -std=gnu99

PKGCONFIG += glib-2.0 nemonotifications-qt5

INCLUDEPATH += /usr/include/libpurple/ \
    /usr/include/glib-2.0/ \
    /usr/lib/glib-2.0/include/

LIBS += -lpurple

SOURCES += \
    src/buddymodel.cpp \
    src/accountsmodel.cpp \
    src/accountsoptionsmodel.cpp \
    src/chatmodel.cpp \
    src/Morsender.cpp \
    src/connection.cpp

HEADERS += \
    src/buddymodel.h \
    src/accountsmodel.h \
    src/accountsoptionsmodel.h \
    src/defines.h \
    src/chatmodel.h \
    src/connection.h

OTHER_FILES += qml/Morsender.qml \
    qml/cover/CoverPage.qml \
    translations/*.ts \
    Morsender.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
#TRANSLATIONS += translations/Morsender-de.ts

DISTFILES += \
    qml/pages/BuddyList.qml \
    qml/pages/AccountsList.qml \
    qml/pages/Chat.qml \
    qml/pages/Settings.qml \
    qml/pages/Share.qml

RESOURCES += \
    qml/resources/resources.qrc

dbus.files = com.mistermagister.morsender.service
dbus.path = /usr/share/dbus-1/services/

systemd.files = Morsender.service
systemd.path = /usr/lib/systemd/user

INSTALLS += dbus systemd
