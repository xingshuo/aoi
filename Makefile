CFLAGS = -g3 -O0 -Wall
SHARED := -fPIC --shared
INC = include
SRC = src
BUILD = build

$(BUILD)/aoi.so: $(SRC)/lua-aoi.c $(SRC)/aoi.c
	gcc $(CFLAGS) $(SHARED) $^ -o $@ -I$(INC)

help:
	@echo "make test1 --single test"
	@echo "make test2 --multi test"

test1:
	bin/lua test/test1.lua
test2:
	bin/lua test/test2.lua

clean:
	rm $(BUILD)/aoi.so