TEMPLATE = app
TARGET = minutor
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += c++11
QT += widgets network
QMAKE_INFO_PLIST = minutor.plist
unix:LIBS += -lz
win32:RC_FILE += winicon.rc
macx:ICON=icon.icns

#for profiling
#*-g++* {
#	QMAKE_CXXFLAGS += -pg
#	QMAKE_LFLAGS += -pg
#}

# Input
HEADERS += \
    zlib/zlib.h \
    zlib/zconf.h \
	  labelledslider.h \
    biomeidentifier.h \
    blockidentifier.h \
    chunk.h \
    chunkcache.h \
    chunkloader.h \
    definitionmanager.h \
    definitionupdater.h \
    dimensionidentifier.h \
    entity.h \
    entityidentifier.h \
    generatedstructure.h \
    json.h \
    mapview.h \
    minutor.h \
    nbt.h \
    overlayitem.h \
    properties.h \
    settings.h \
    village.h \
    worldsave.h \
    zipreader.h \
    clamp.h
SOURCES += \
	  labelledslider.cpp \
    biomeidentifier.cpp \
    blockidentifier.cpp \
    chunk.cpp \
    chunkcache.cpp \
    chunkloader.cpp \
    definitionmanager.cpp \
    definitionupdater.cpp \
    dimensionidentifier.cpp \
    entity.cpp \
    entityidentifier.cpp \
    generatedstructure.cpp \
    json.cpp \
    main.cpp \
    mapview.cpp \
    minutor.cpp \
    nbt.cpp \
    properties.cpp \
    settings.cpp \
    village.cpp \
    worldsave.cpp \
    zipreader.cpp
RESOURCES = minutor.qrc

win32:SOURCES += zlib/adler32.c \
		zlib/compress.c \
		zlib/crc32.c \
		zlib/deflate.c \
		zlib/gzclose.c \
		zlib/gzlib.c \
		zlib/gzread.c \
		zlib/gzwrite.c \
		zlib/infback.c \
		zlib/inffast.c \
		zlib/inflate.c \
		zlib/inftrees.c \
		zlib/trees.c \
		zlib/uncompr.c \
		zlib/zutil.c

desktopfile.path = /usr/share/applications
desktopfile.files = minutor.desktop
pixmapfile.path = /usr/share/pixmaps
pixmapfile.files = minutor.png minutor.xpm
target.path = /usr/bin
INSTALLS += desktopfile pixmapfile target

FORMS += \
    properties.ui \
    settings.ui
