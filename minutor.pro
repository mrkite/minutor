TEMPLATE = app
TARGET = minutor
DEPENDPATH += .
INCLUDEPATH += .
QT += widgets network
QMAKE_INFO_PLIST = minutor.plist
unix:LIBS += -lz
win32:LIBS+= $$PWD/zlibstat.lib
win32:RC_FILE += winicon.rc
macx:ICON=icon.icns

# Input
HEADERS += mapview.h minutor.h nbt.h zlib.h zconf.h \
	labelledslider.h \
    chunk.h \
    chunkcache.h \
    json.h \
    blockidentifier.h \
    biomeidentifier.h \
    dimensions.h \
    definitionmanager.h \
    zipreader.h \
    settings.h \
    chunkloader.h \
    definitionupdater.h \
    worldsave.h \
    entity.h \
    properties.h
SOURCES += mapview.cpp main.cpp minutor.cpp nbt.cpp \
	labelledslider.cpp \
    chunk.cpp \
    chunkcache.cpp \
    json.cpp \
    blockidentifier.cpp \
    biomeidentifier.cpp \
    dimensions.cpp \
    definitionmanager.cpp \
    zipreader.cpp \
    settings.cpp \
    chunkloader.cpp \
    definitionupdater.cpp \
    worldsave.cpp \
    entity.cpp \
    properties.cpp
RESOURCES = minutor.qrc

desktopfile.path = /usr/share/applications
desktopfile.files = minutor.desktop
pixmapfile.path = /usr/share/pixmaps
pixmapfile.files = minutor.png minutor.xpm
target.path = /usr/bin
INSTALLS += desktopfile pixmapfile target

FORMS += \
    properties.ui \
    settings.ui
