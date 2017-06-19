CFLAGS = -g -O2 -Wall
SHARED := -fPIC --shared
INC = include
SRC = src

aoi.so: $(SRC)/lua-aoi.c $(SRC)/aoi.c
	gcc $(CFLAGS) $(SHARED) $^ -o $@ -I$(INC)

run:
	bin/lua test.lua

clean:
	rm aoi.so