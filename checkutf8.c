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

/*
 * Some futher optimizations
 * - Replace logical operators with bitwise ones, to eliminate some branches
 *   - Probably won't improve things much since these branches are easy to
 *     predict
 * - Remove extra boundary checks in the loop by ensuring accessing at most
 *   three bytes past the memory boundary is OK.
 * - Add a fast path to process likely ASCII stuff in batch, utilizing the full
 *   machine register (4/8 bytes).
 */

// Trailing bytes are in range [0x80, 0xbf], or [0b1000, 0b1011], i.e. their
// most significant two bits must be 0b10.
#define valid_trailer(c)	(((c) & 0xc0) == 0x80)

static size_t
utf8_validate(const char *str, size_t len)
{
	const uint8_t *p = (const uint8_t *)str;
	size_t i = 0;

	while (i < len) {
		if (p[i] <= 0x7f) {
			/* [00, 7f] */
			i++;
		} else if (p[i] >= 0xc2 && p[i] <= 0xdf) {
			/* [c2 - df] [80 - bf] */
			if (len - i < 2)
				return i;

			if (!valid_trailer(p[i + 1]))
				return i;

			i += 2;
		} else if (p[i] >= 0xe0 && p[i] <= 0xef) {
			/*
			 * [e0]     [a0, bf] [80, bf]
			 * [e1, ec] [80, bf] [80, bf]
			 * [ed]     [80, 9f] [80, bf]
			 * [ee, ef] [80, bf] [80, bf]
			 */
			if (len - i < 3)
				return i;

			if (!valid_trailer(p[i + 1]) ||
			    !valid_trailer(p[i + 2]))
				return i;

			// For sequences starting with 0xe0 and 0xed
			if ((p[i] == 0xe0 && p[i + 1] < 0xa0) ||
			    (p[i] == 0xed && p[i + 1] > 0x9f))
				return i;

			i += 3;
		} else if (p[i] >= 0xf0 && p[i] <= 0xf4) {
			/*
			 * [f0]     [90, bf] [80, bf] [80, bf]
			 * [f1, f3] [80, bf] [80, bf] [80, bf]
			 * [f4]     [80, 8f] [80, bf] [80, bf]
			 */
			if (len - i < 4)
				return i;

			if (!valid_trailer(p[i + 1]) ||
			    !valid_trailer(p[i + 2]) ||
			    !valid_trailer(p[i + 3]))
				return i;

			// For sequences starting with 0xf0 and 0xf4
			if ((p[i] == 0xf0 && p[i + 1] < 0x90) ||
			    (p[i] == 0xf4 && p[i + 1] > 0x8f))
				return i;

			i += 4;
		} else {
			return i;
		}
	}

	return i;
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
