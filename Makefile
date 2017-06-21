CFLAGS = -g3 -Wall
SHARED := -fPIC --shared
INC = include
SRC = src
BUILD = build

all: $(BUILD)/aoi.so $(BUILD)/twheel.so

$(BUILD)/aoi.so: $(SRC)/lua-aoi.c $(SRC)/aoi.c
	gcc $(CFLAGS) $(SHARED) $^ -o $@ -I$(INC)

$(BUILD)/twheel.so: 3rd/twheel.c
	gcc $(CFLAGS) $(SHARED) $^ -o $@ -I$(INC)

help:
	@echo "make --generate the build/aoi.so"
	@echo "make clean --delete the build/aoi.so"
	@echo "make test1 --run test/test1.lua"
	@echo "make test2 --run test/test2.lua"

test1 test2:
	bin/lua test/$@.lua

clean:
	rm $(BUILD)/aoi.so