#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"

#define INVALID_ID (~0)
#define PRE_ALLOC 2
#define check_aoi(L, idx)\
    *(map**)luaL_checkudata(L, idx, "aoi_meta")

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

static tower *
get_tower(map * m, int row, int col) {
    if (!(row >= 0 && row < m->max_row && col >= 0 && col < m->max_col)) {
        return NULL;
    }
    int index = row*m->max_col + col;
    if (m->tower_list[index] == NULL) {
        m->tower_list[index] = new_tower(row, col);
    }
    return m->tower_list[index];
};

static void
insert_obj_to_tower(tower* t, object* obj) {
    obj->pNext = t->pHead->pNext;
    obj->pPrev = t->pHead;
    obj->pNext->pPrev = obj;
    obj->pPrev->pNext = obj;
    obj->pTower = t;
}

static void
delete_obj_from_tower(tower* t, object* obj) {
    obj->pNext->pPrev = obj->pPrev;
    obj->pPrev->pNext = obj->pNext;
    obj->pPrev = NULL;
    obj->pNext = NULL;
    obj->pTower = NULL;
}

static object *
new_object(map * m, uint64_t id) {
    object * obj = malloc(sizeof(*obj));
    obj->id = id;
    obj->pTower = NULL;
    return obj;
}

static object *
delete_object(map *m, uint64_t id){
    uint64_t hash = id & (m->size-1);
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

static inline slot *
mainposition(map *m , uint64_t id) {
    uint64_t hash = id & (m->size-1);
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
        }
    }
    free(old_slot);
}

static object *
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

static object *
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

static int
map_new(lua_State* L) {
    int max_x = luaL_checknumber(L, 1);
    int max_z = luaL_checknumber(L, 2);
    int view_x = luaL_checknumber(L, 3);
    int view_z = luaL_checknumber(L, 4);
    int aoi_r = luaL_checknumber(L, 5);
    map * m = malloc(sizeof(*m));
    m->size = PRE_ALLOC;
    m->lastfree = PRE_ALLOC - 1;
    m->max_row = ceil(max_z/view_z);
    m->max_col = ceil(max_x/view_x);
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
    *(map**)lua_newuserdata(L, sizeof(void*)) = m;
    luaL_getmetatable(L, "aoi_meta");
    lua_setmetatable(L, -2);
    return 1;
};

static void
map_delete(lua_State* L) {
    printf("1111111111\n");
    map* m = check_aoi(L, 1);
    int i;
    for (i=0;i<m->size;i++) {
        slot * s = &m->slot_list[i];
        if (s->obj) {
            free(s->obj);
            s->obj = NULL;
        }
    }
    printf("22222222222\n");
    free(m->slot_list);
    printf("6666666666\n");
    for(i=0; i<m->max_row*m->max_col; i++){
        if (m->tower_list[i]){
            object* pHead = m->tower_list[i]->pHead;
            object* pCur = pHead->pNext;
            while (pCur != pHead) {
                object* temp = pCur;
                pCur = pCur->pNext;
                free(temp);
            }
            free(pHead);
            free(m->tower_list[i]);
        }
    }
    printf("00000000000\n");
    free(m->tower_list);
    printf("44444444444\n");
    free(m);
    printf("33333333333\n");
}

static int
aoi_add(lua_State* L) {
    map* m = check_aoi(L, 1);
    uint64_t id = luaL_checkinteger(L, 2);
    float x = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);
    object * obj = map_query_object(m, id);
    if (obj) {
        return 0;
    }
    obj = map_init_object(m, id);
    obj->x = x;
    obj->z = z;
    int row = z/m->view_z;
    int col = x/m->view_x;
    tower *t = get_tower(m, row, col);
    if (!t) {
        return 0;
    }
    insert_obj_to_tower(t, obj);
    lua_newtable(L);
    int i,j;
    for (i=-m->aoi_r; i<=m->aoi_r; i++) {
        for (j=-m->aoi_r; j<=m->aoi_r; j++){
            int r = row + i;
            int c = col + j;
            t = get_tower(m, r, c);
            if (!t) {
                continue;
            }
            object* pCur = t->pHead->pNext;
            while (pCur != t->pHead) {
                if (pCur->id != id) {
                    lua_pushinteger(L,1);
                    lua_rawseti(L,-2,pCur->id);
                }
                pCur = pCur->pNext;
            }
        }
    }
    return 1;
}

static int
aoi_update(lua_State* L) {
    map* m = check_aoi(L, 1);
    uint64_t id = luaL_checkinteger(L, 2);
    float x = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);
    object * obj = map_query_object(m, id);
    if (!obj) {
        return 0;
    }
    if (obj->x == x && obj->z == z) {
        return 0;
    }
    int old_row = obj->z/m->view_z;
    int old_col = obj->x/m->view_x;
    obj->x = x;
    obj->z = z;
    int new_row = z/m->view_z;
    int new_col = x/m->view_x;
    if (new_row == old_row && new_col == old_col) {
        return 0;
    }
    delete_obj_from_tower(NULL, obj);

    tower* new_t = get_tower(m, new_row, new_col);
    insert_obj_to_tower(new_t, obj);

    lua_newtable(L); //the return table

    lua_pushinteger(L,1);
    lua_newtable(L); //leave_list
    int i,j;
    for (i=-m->aoi_r; i<=m->aoi_r; i++) {
        for (j=-m->aoi_r; j<=m->aoi_r; j++){
            int r = old_row + i;
            int c = old_col + j;
            tower* t = get_tower(m, r, c);
            if (!t) {
                continue;
            }
            if ((abs(new_row-r)>m->aoi_r) || (abs(new_col-c)>m->aoi_r)) {
                object* pCur = t->pHead->pNext;
                while (pCur != t->pHead) {
                    if (pCur->id != id) {
                        lua_pushinteger(L,1);
                        lua_rawseti(L,-2,pCur->id);
                    }
                    pCur = pCur->pNext;
                }
            }
        }
    }
    lua_rawset(L, -3);

    lua_pushinteger(L,2);
    lua_newtable(L); //enter_list
    for (i=-m->aoi_r; i<=m->aoi_r; i++) {
        for (j=-m->aoi_r; j<=m->aoi_r; j++){
            int r = new_row + i;
            int c = new_col + j;
            tower* t = get_tower(m, r, c);
            if (!t) {
                continue;
            }
            if ((abs(old_row-r)>m->aoi_r) || (abs(old_col-c)>m->aoi_r)) {
                object* pCur = t->pHead->pNext;
                while (pCur != t->pHead) {
                    if (pCur->id != id) {
                        lua_pushinteger(L,1);
                        lua_rawseti(L,-2,pCur->id);
                    }
                    pCur = pCur->pNext;
                }
            }
        }
    }
    lua_rawset(L, -3);
    return 1;
}

static int
aoi_delete(lua_State* L) {
    map* m = check_aoi(L, 1);
    uint64_t id = luaL_checkinteger(L, 2);
    lua_newtable(L);
    object * obj = map_query_object(m, id);
    if (!obj) {
        return 1;
    }
    int row = obj->pTower->row;
    int col = obj->pTower->col;
    int i,j;
    for (i=-m->aoi_r; i<=m->aoi_r; i++) {
        for (j=-m->aoi_r; j<=m->aoi_r; j++){
            int r = row + i;
            int c = col + j;
            tower* t = get_tower(m, r, c);
            if (!t) {
                continue;
            }
            object* pCur = t->pHead->pNext;
            while (pCur != t->pHead) {
                if (pCur->id != id) {
                    lua_pushinteger(L,1);
                    lua_rawseti(L,-2,pCur->id);
                }
                pCur = pCur->pNext;
            }
        }
    }
    delete_object(m, id);
    return 1;
}

static int
aoi_get_pos_nearby_objs(lua_State* L) {
    map* m = check_aoi(L, 1);
    float x = luaL_checknumber(L, 2);
    float z = luaL_checknumber(L, 3);
    int aoi_r = luaL_checkinteger(L, 4);
    int row = z/m->view_z;
    int col = x/m->view_x;
    lua_newtable(L);
    tower* t = get_tower(m, row, col);
    if (!t){
        return 1;
    }
    int i,j;
    for (i=-aoi_r; i<=aoi_r; i++) {
        for (j=-aoi_r; j<=aoi_r; j++){
            int r = row + i;
            int c = col + j;
            t = get_tower(m, r, c);
            if (!t) {
                continue;
            }
            object* pCur = t->pHead->pNext;
            while (pCur != t->pHead) {
                lua_pushinteger(L,1);
                lua_rawseti(L,-2,pCur->id);
                pCur = pCur->pNext;
            }
        }
    }
    return 1;
}

static int
aoi_get_obj_nearby_objs(lua_State* L) {
    map* m = check_aoi(L, 1);
    uint64_t id = luaL_checkinteger(L, 2);
    int aoi_r = luaL_checkinteger(L, 3);
    lua_newtable(L);
    object* obj = map_query_object(m, id);
    if (!obj) {
        return 1;
    }
    int row = obj->pTower->row;
    int col = obj->pTower->col;
    int i,j;
    for (i=-aoi_r; i<=aoi_r; i++) {
        for (j=-aoi_r; j<=aoi_r; j++){
            int r = row + i;
            int c = col + j;
            tower* t = get_tower(m, r, c);
            if (!t) {
                continue;
            }
            object* pCur = t->pHead->pNext;
            while (pCur != t->pHead) {
                lua_pushinteger(L,1);
                lua_rawseti(L,-2,pCur->id);
                pCur = pCur->pNext;
            }
        }
    }
    return 1;
}

static int
aoi_get_objs_nearby_objs(lua_State* L) {
    map* m = check_aoi(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    int aoi_r = luaL_checkinteger(L, 3);
    lua_newtable(L);
    lua_pushnil(L);
    while (lua_next(L, 2) != 0){
        uint64_t id = luaL_checkinteger(L, -1);
        object* obj = map_query_object(m, id);
        if (obj){
            int row = obj->pTower->row;
            int col = obj->pTower->col;
            int i,j;
            for (i=-aoi_r; i<=aoi_r; i++) {
                for (j=-aoi_r; j<=aoi_r; j++){
                    int r = row + i;
                    int c = col + j;
                    tower* t = get_tower(m, r, c);
                    if (!t) {
                        continue;
                    }
                    object* pCur = t->pHead->pNext;
                    while (pCur != t->pHead) {
                        lua_pushinteger(L,1);
                        lua_rawseti(L,-4,pCur->id);
                        pCur = pCur->pNext;
                    }
                }
            }
        }
        lua_pop(L, 1);
    }
    return 1;
}

int luaopen_aoi(lua_State* L) {
    luaL_checkversion(L);
    luaL_Reg l1[] = {
        {"create", map_new},
        {NULL, NULL},
    };
    luaL_Reg l2[] = {
        {"add", aoi_add},
        {"update", aoi_update},
        {"delete", aoi_delete},
        {"get_pos_nearby_objs", aoi_get_pos_nearby_objs},
        {"get_obj_nearby_objs", aoi_get_obj_nearby_objs},
        {"get_objs_nearby_objs", aoi_get_objs_nearby_objs},
        {NULL, NULL},
    };
    luaL_newmetatable(L, "aoi_meta");
    luaL_newlib(L, l2);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, map_delete);
    lua_setfield(L, -2, "__gc");

    luaL_newlib(L, l1);
    return 1;
}