TEMPLATE = app
TARGET = minutor
CONFIG += c++14
QT += widgets network concurrent
QMAKE_INFO_PLIST = minutor.plist
unix:LIBS += -lz
win32:RC_FILE += winicon.rc
macx:ICON=icon.icns

#for profiling
#*-g++* {
#    QMAKE_CXXFLAGS += -pg
#    QMAKE_LFLAGS += -pg
#}

# Input
HEADERS += \
    chunkid.h \
    chunkmath.h \
    entityevaluator.h \
    labelledseparator.h \
    labelledslider.h \
    clamp.h \
    chunk.h \
    chunkcache.h \
    chunkloader.h \
    chunkrenderer.h \
    identifier\biomeidentifier.h \
    identifier\blockidentifier.h \
    identifier\definitionmanager.h \
    identifier\definitionupdater.h \
    identifier\dimensionidentifier.h \
    identifier\entityidentifier.h \
    identifier\flatteningconverter.h \
    jumpto.h \
    json\json.h \
    mapview.h \
    minutor.h \
    nbt\nbt.h \
    nbt\tag.h \
    nbt\tagdatastream.h \
    overlay\entity.h \
    overlay\generatedstructure.h \
    overlay\overlayitem.h \
    overlay\village.h \
    paletteentry.h \
    pngexport.h \
    properties.h \
    propertietreecreator.h \
    range.h \
    rectangleinnertoouteriterator.h \
    searchblockpluginwidget.h \
    searchchunkswidget.h \
    searchentitypluginwidget.h \
    searchplugininterface.h \
    searchresultwidget.h \
    searchtextwidget.h \
    settings.h \
    worldsave.h \
    zipreader.h
SOURCES += \
    entityevaluator.cpp \
    labelledseparator.cpp \
    labelledslider.cpp \
    chunk.cpp \
    chunkcache.cpp \
    chunkloader.cpp \
    chunkrenderer.cpp \
    identifier\biomeidentifier.cpp \
    identifier\blockidentifier.cpp \
    identifier\definitionmanager.cpp \
    identifier\definitionupdater.cpp \
    identifier\dimensionidentifier.cpp \
    identifier\entityidentifier.cpp \
    identifier\flatteningconverter.cpp \
    jumpto.cpp \
    json\json.cpp \
    main.cpp \
    mapview.cpp \
    minutor.cpp \
    nbt\nbt.cpp \
    nbt\tag.cpp \
    nbt\tagdatastream.cpp \
    overlay\entity.cpp \
    overlay\generatedstructure.cpp \
    overlay\village.cpp \
    pngexport.cpp \
    properties.cpp \
    propertietreecreator.cpp \
    searchblockpluginwidget.cpp \
    searchchunkswidget.cpp \
    searchentitypluginwidget.cpp \
    searchresultwidget.cpp \
    searchtextwidget.cpp \
    settings.cpp \
    worldsave.cpp \
    zipreader.cpp
RESOURCES = minutor.qrc

win32 {
HEADERS += \
    zlib/crc32.h \
    zlib/deflate.h \
    zlib/gzguts.h \
    zlib/inffast.h \
    zlib/inffixed.h \
    zlib/inflate.h \
    zlib/inftrees.h \
    zlib/trees.h \
    zlib/zconf.h \
    zlib/zlib.h \
    zlib/zutil.h

SOURCES += \
    zlib/adler32.c \
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

INCLUDEPATH += zlib
}

desktopfile.path = /usr/share/applications
desktopfile.files = minutor.desktop
pixmapfile.path = /usr/share/pixmaps
pixmapfile.files = minutor.png minutor.xpm
target.path = /usr/bin
INSTALLS += desktopfile pixmapfile target

FORMS += \
    minutor.ui \
    properties.ui \
    searchchunkswidget.ui \
    searchresultwidget.ui \
    searchtextwidget.ui \
    settings.ui \
    jumpto.ui \
    pngexport.ui
