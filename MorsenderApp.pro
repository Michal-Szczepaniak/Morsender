TEMPLATE = subdirs

SUBDIRS = \
    Morsender \
    MorsenderSharePlugin

OTHER_FILES += rpm/Morsender.spec

DISTFILES += \
    ../ambienceCreator/qml/pages/spColorPicker.qml
