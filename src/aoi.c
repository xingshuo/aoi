#include "aoi.h"
#include <assert.h>

#define INVALID_ID (~0)
#define PRE_ALLOC 2

static tower *
new_tower(int row, int col){
    tower * t = malloc(sizeof(*t));
    t->row = row;
    t->col = col;
    t->pHead = malloc(sizeof(object));
    t->pHead->pNext = t->pHead;
    t->pHead->pPrev = t->pHead;
    return t;
};

static object *
new_object(map * m, uint64_t id) {
    object * obj = malloc(sizeof(*obj));
    obj->id = id;
    obj->pTower = NULL;
    obj->pPrev = NULL;
    obj->pNext = NULL;
    return obj;
}

static inline slot *
mainposition(map *m , uint64_t id) {
    int hash = id & (m->size-1);
    return &m->slot_list[hash];
}

static void rehash(map *m);

static void
map_insert(map * m, uint64_t id, object *obj) {
    slot *s = mainposition(m,id);
    if (s->id == INVALID_ID) {
        s->id = id;
        s->obj = obj;
        return;
    }
    slot *last = mainposition(m, s->id);
    if (last != s) {
        while (last->next != s - m->slot_list) {
            assert(last->next >= 0);
            last = &m->slot_list[last->next];
        }
        uint64_t temp_id = s->id;
        object *temp_obj = s->obj;
        last->next = s->next;
        s->id = id;
        s->obj = obj;
        s->next = -1;
        if (temp_obj) {
            map_insert(m, temp_id, temp_obj);
        }
        return;
    }
    while (m->lastfree >= 0) {
        slot * temp = &m->slot_list[m->lastfree--];
        if (temp->id == INVALID_ID) {
            temp->id = id;
            temp->obj = obj;
            temp->next = s->next;
            s->next = (int)(temp - m->slot_list);
            return;
        }
    }
    rehash(m);
    map_insert(m, id , obj);
}

static void
rehash(map * m) {
    slot * old_slot = m->slot_list;
    int old_size = m->size;
    m->size = 2 * old_size;
    m->lastfree = m->size - 1;
    m->slot_list = malloc(m->size * sizeof(slot));
    int i;
    for (i=0;i<m->size;i++) {
        slot * s = &m->slot_list[i];
        s->id = INVALID_ID;
        s->obj = NULL;
        s->next = -1;
    }
    for (i=0;i<old_size;i++) {
        slot * s = &old_slot[i];
        if (s->obj) {
            map_insert(m, s->id, s->obj);
            s->obj = NULL;
        }
    }
    free(old_slot);
}

tower *
get_tower(map * m, int row, int col) {
    if (!(row >= 0 && row < m->max_row && col >= 0 && col < m->max_col)) {
        return NULL;
    }
    int index = row*m->max_col + col;
    assert(index < (m->max_row*m->max_col));
    if (m->tower_list[index] == NULL) {
        m->tower_list[index] = new_tower(row, col);
    }
    return m->tower_list[index];
};

void
insert_obj_to_tower(tower* t, object* obj) {
    obj->pNext = t->pHead->pNext;
    obj->pPrev = t->pHead;
    obj->pNext->pPrev = obj;
    obj->pPrev->pNext = obj;
    obj->pTower = t;
}

void
delete_obj_from_tower(tower* t, object* obj) {
    obj->pNext->pPrev = obj->pPrev;
    obj->pPrev->pNext = obj->pNext;
    obj->pPrev = NULL;
    obj->pNext = NULL;
    obj->pTower = NULL;
}


object *
map_init_object(map * m, uint64_t id){
    slot *s = mainposition(m, id);
    for (;;) {
        if (s->id == id) {
            if (s->obj == NULL) {
                s->obj = new_object(m, id);
            }
            return s->obj;
        }
        if (s->next < 0) {
            break;
        }
        s=&m->slot_list[s->next];
    }
    object * obj = new_object(m, id);
    map_insert(m, id , obj);
    return obj;
}

object *
map_query_object(map * m, uint64_t id){
    slot *s = mainposition(m, id);
    for (;;) {
        if (s->id == id) {
            return s->obj;
        }
        if (s->next < 0) {
            return NULL;
        }
        s=&m->slot_list[s->next];
    }
}

object *
delete_object(map *m, uint64_t id){
    int hash = id & (m->size-1);
    slot *s = &m->slot_list[hash];
    for (;;) {
        if (s->id == id) {
            object * obj = s->obj;
            s->obj = NULL;
            delete_obj_from_tower(NULL, obj);
            free(obj);
            return obj;
        }
        if (s->next < 0) {
            return NULL;
        }
        s=&m->slot_list[s->next];
    }
}

map* 
map_new(int max_x, int max_z, int view_x, int view_z, int aoi_r){
    map * m = malloc(sizeof(*m));
    m->size = PRE_ALLOC;
    m->lastfree = PRE_ALLOC - 1;
    m->max_row = max_z/view_z;
    if (max_z%view_z != 0){
        m->max_row += 1;
    }
    m->max_col = max_x/view_x;
    if (max_x%view_x != 0){
        m->max_col += 1;
    }
    m->view_x = view_x;
    m->view_z = view_z;
    m->aoi_r = aoi_r;
    m->slot_list = malloc(m->size * sizeof(slot));
    int i;
    for (i=0;i<m->size;i++) {
        slot * s = &m->slot_list[i];
        s->id = INVALID_ID;
        s->obj = NULL;
        s->next = -1;
    }
    m->tower_list = calloc(m->max_row * m->max_col, sizeof(tower *));
    return m;
}

void
map_delete(map* m){
    int i;
    for (i=0;i<m->size;i++) {
        slot * s = &m->slot_list[i];
        if (s->obj) {
            free(s->obj);
            s->obj = NULL;
        }
    }
    free(m->slot_list);
    for(i=0; i<m->max_row*m->max_col; i++){
        if (m->tower_list[i]){
            object* pHead = m->tower_list[i]->pHead;
            free(pHead);
            free(m->tower_list[i]);
        }
    }
    free(m->tower_list);
    free(m);
}