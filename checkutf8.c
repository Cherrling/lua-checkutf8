// SPDX-License-Identifier: MPL-2.0
/*
 * checkutf8 - Module for validating and repairing UTF-8 sequences.
 * Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <lua.h>
#include <lauxlib.h>

#define UTF8_VALIDATE utf8_validate_fast
#include "checkutf8-impl.h"

#undef UTF8_VALIDATE
#define BOUNDARY_CHECK
#define UTF8_VALIDATE utf8_validate_checked
#include "checkutf8-impl.h"

static size_t
utf8_validate(const char *str, size_t len)
{
	size_t err = 0;

	/* Short inputs fit in one pass through the checked loop. */
	if (len >= 16) {
		err = utf8_validate_fast(str, len - 4);
		str += err;
		len -= err;
	}

	return err + utf8_validate_checked(str, len);
}

static int
checkutf8_validate(lua_State *l)
{
	luaL_checkstring(l, 1);

	size_t len;
	const char *s = lua_tolstring(l, 1, &len);

	size_t err = utf8_validate(s, len);

	if (err == len) {
		lua_pushboolean(l, 1);
		return 1;
	}

	lua_pushboolean(l, 0);
	lua_pushinteger(l, (lua_Integer)err + 1);

	return 2;
}

static int
checkutf8_repair(lua_State *l)
{
	luaL_checkstring(l, 1);

	size_t len;
	const char *s = lua_tolstring(l, 1, &len);

	size_t replacementLen;
	const char *replacement = luaL_optlstring(l, 2, "\xef\xbf\xbd",
						  &replacementLen);

	size_t err = utf8_validate(s, len);
	if (err == len) {
		lua_settop(l, 1);
		return 1;
	}

	luaL_Buffer buff;
	luaL_buffinit(l, &buff);

	luaL_addlstring(&buff, s, err);
	luaL_addlstring(&buff, replacement, replacementLen);

	size_t start = err + 1;
	for (err = utf8_validate(s + start, len - start);
	     start + err < len;
	     err = utf8_validate(s + start, len - start)) {
		luaL_addlstring(&buff, s + start, err);
		luaL_addlstring(&buff, replacement, replacementLen);
		start += err + 1;
	}

	luaL_addlstring(&buff, s + start, len - start);

	luaL_pushresult(&buff);

	return 1;
}

static const luaL_Reg checkutf8_funcs[] = {
	{ "validate", checkutf8_validate },
	{ "repair", checkutf8_repair },
	{ NULL, NULL },
};

int
luaopen_checkutf8(lua_State *l)
{
	luaL_newlib(l, checkutf8_funcs);

	return 1;
}
