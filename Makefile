CFLAGS = -g3 -O2 -Wall
SHARED := -fPIC --shared
INC = include
SRC = src
BUILD = build

$(BUILD)/aoi.so: $(SRC)/lua-aoi.c $(SRC)/aoi.c
	gcc $(CFLAGS) $(SHARED) $^ -o $@ -I$(INC)

run:
	bin/lua test/run.lua

clean:
	rm $(BUILD)/aoi.so