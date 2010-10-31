GTK_INC=`pkg-config --cflags gtk+-2.0`
GTK_LIB=`pkg-config --libs gtk+-2.0`

BIN=$(DESTDIR)/usr/bin
DESKTOP=$(DESTDIR)/usr/share/applications
PIXMAP=$(DESTDIR)/usr/share/pixmaps
SCHEMAS=$(DESTDIR)/usr/share/glib-2.0/schemas
OBJS=minutor.o mapviewer.o colorschemes.o
CC=gcc
MAKE=make
CFLAGS=-Wall -std=c99 -O3
INCLUDE=$(GTK_INC)
LIBS=$(GTK_LIB)

EXEC=minutor

all: $(EXEC)

$(EXEC): $(OBJS) maplib
	$(CC) -Wl,--as-needed -o $(EXEC) $(OBJS) $(LIBS) MinutorMap/MinutorMap.a

install: $(EXEC)
	install -d $(BIN) $(DESKTOP) $(PIXMAP) $(SCHEMAS)
	install -s $(EXEC) $(BIN)
	install minutor.desktop $(DESKTOP)
	install minutor.png $(PIXMAP)
	install minutor.xpm $(PIXMAP)
	install com.seancode.minutor.gschema.xml $(SCHEMAS)

maplib:
	cd MinutorMap && $(MAKE)

%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $< $(INCLUDE)

clean: cleanmaplib
	rm -f $(EXEC) $(OBJS)
cleanmaplib:
	cd MinutorMap && $(MAKE) clean
