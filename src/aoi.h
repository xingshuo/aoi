#ifndef _AOI_H
#define _AOI_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

typedef struct object {
    uint64_t id;
    float x;
    float z;
    struct object * pNext;
    struct object * pPrev;
    struct tower * pTower;
} object;

typedef struct tower {
    int row;
    int col;
    object * pHead;
} tower;

typedef struct slot {
    uint64_t id;
    object * obj;
    int next;
} slot;

typedef struct map {
    int size;
    int lastfree;
    slot * slot_list;
    int max_row;
    int max_col;
    int view_x;
    int view_z;
    int aoi_r;
    tower ** tower_list;
} map;

map* map_new(int, int, int, int, int);
void map_delete(map*);
object* map_query_object(map *, uint64_t);
object* map_init_object(map *, uint64_t);
object* delete_object(map *, uint64_t);
tower* get_tower(map*, int, int);
void insert_obj_to_tower(tower*, object*);
void delete_obj_from_tower(tower*, object*);

#endif