CFLAGS = -g -Wall
SHARED := -fPIC --shared

aoi.so: aoi.c
	gcc $(CFLAGS) $(SHARED) $^ -o $@

run:
	lua test.lua

clean:
	rm aoi.so