GTK_INC=`pkg-config --cflags gtk+-2.0`
GTK_LIB=`pkg-config --libs gtk+-2.0`

BIN=$(DESTDIR)/usr/bin
DESKTOP=$(DESTDIR)/usr/share/applications
PIXMAP=$(DESTDIR)/usr/share/pixmaps
OBJS=minutor.o mapviewer.o colorschemes.o
CC=gcc
MAKE=make
#CFLAGS=-Wall -std=c99 -O2
CFLAGS=-Wall -std=c99 -g
INCLUDE=$(GTK_INC)
LIBS=$(GTK_LIB)

EXEC=minutor

all: $(EXEC)

$(EXEC): $(OBJS) maplib
	$(CC) -Wl,--as-needed -o $(EXEC) $(OBJS) $(LIBS) MinutorMap/MinutorMap.a

install: $(EXEC)
	install -d $(BIN) $(DESKTOP) $(PIXMAP)
	install -s $(EXEC) $(BIN)
	install minutor.desktop $(DESKTOP)
	install minutor.png $(PIXMAP)
	install minutor.xpm $(PIXMAP)

maplib:
	cd MinutorMap && $(MAKE)

%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $< $(INCLUDE)

clean: cleanmaplib
	rm -f $(EXEC) $(OBJS)
cleanmaplib:
	cd MinutorMap && $(MAKE) clean