OBJS=MinutorMap.o cache.o nbt.o region.o
CC=gcc
AR=ar
#CFLAGS=-Wall -std=c99 -O2
CFLAGS=-Wall -std=c99 -g

LIB=MinutorMap.a

$(LIB): $(OBJS)
	$(AR) -cr $(LIB) $(OBJS)
%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $< $(INCLUDE)
clean:
	rm -f $(LIB) $(OBJS)
