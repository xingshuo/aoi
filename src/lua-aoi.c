#include "aoi.h"
#include "lua.h"
#include "lauxlib.h"

#define check_aoi(L, idx)\
    *(map**)luaL_checkudata(L, idx, "aoi_meta")

static int
aoi_new(lua_State* L) {
    int max_x = luaL_checknumber(L, 1);
    int max_z = luaL_checknumber(L, 2);
    int view_x = luaL_checknumber(L, 3);
    int view_z = luaL_checknumber(L, 4);
    int aoi_r = luaL_checknumber(L, 5);
    map * m = map_new(max_x, max_z, view_x, view_z, aoi_r);
    *(map**)lua_newuserdata(L, sizeof(void*)) = m;
    luaL_getmetatable(L, "aoi_meta");
    lua_setmetatable(L, -2);
    return 1;
};

static int
aoi_release(lua_State* L) {
    map* m = check_aoi(L, 1);
    map_delete(m);
    return 0;
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
        {"create", aoi_new},
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
    lua_pushcfunction(L, aoi_release);
    lua_setfield(L, -2, "__gc");

    luaL_newlib(L, l1);
    return 1;
}