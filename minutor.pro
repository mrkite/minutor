TEMPLATE = app
TARGET = minutor
DEPENDPATH += .
INCLUDEPATH += .
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
    properties.h \
    generatedstructure.h \
    overlayitem.h \
    village.h
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
    properties.cpp \
    generatedstructure.cpp \
    village.cpp
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
