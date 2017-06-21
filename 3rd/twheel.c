#include <time.h>
#include <sys/time.h>
#include "lua.h"
#include "lauxlib.h"

static double
_gettime(void)  {
    struct timeval v;
    gettimeofday(&v, (struct timezone *) NULL);
    /* Unix Epoch time (time since January 1, 1970 (UTC)) */
    return v.tv_sec + v.tv_usec/1.0e6;
}

static int
lgettime(lua_State* L) {
	double t = _gettime();
	lua_pushnumber(L, t);
	return 1;
}

int luaopen_twheel(lua_State* L) {
	luaL_checkversion(L);
	luaL_Reg l1[] = {
		{"gettime", lgettime},
		{NULL, NULL},
	};
	luaL_newlib(L, l1);
	return 1;
}