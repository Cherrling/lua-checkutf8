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

#ifndef __SSE2__
#define UTF8_VALIDATE utf8_validate_fast
#include "checkutf8-impl.h"

#undef UTF8_VALIDATE
#endif
#define BOUNDARY_CHECK
#define UTF8_VALIDATE utf8_validate_checked
#include "checkutf8-impl.h"

#ifdef __SSE2__
#include <emmintrin.h>

static inline __m128i
load16(const char *p)
{
	return _mm_loadu_si128((const __m128i_u *)p);
}

/* ASCII also compares above these negative signed-byte thresholds. */
static inline unsigned
signed_byte_mask_at_least(__m128i v, unsigned byte)
{
	__m128i threshold = _mm_set1_epi8((char)(byte - 1));
	return _mm_movemask_epi8(_mm_cmpgt_epi8(v, threshold));
}

/* Return the length of a validated prefix, ending on a character boundary. */
static size_t
utf8_validate_blocks(const char *str, size_t len)
{
	size_t i = 0;
	while (len - i >= 16) {
		__m128i v = load16(str + i);
		unsigned high = _mm_movemask_epi8(v);
		if (!high) {
			i += 16;
			while (len - i >= 64) {
				const char *p = str + i;
				__m128i a = _mm_or_si128(load16(p), load16(p + 16));
				__m128i b = _mm_or_si128(load16(p + 32), load16(p + 48));
				if (_mm_movemask_epi8(_mm_or_si128(a, b)))
					break;
				i += 64;
			}
			continue;
		}

		/* Each leading byte requires one, two or three trailers. */
		unsigned lead = high & signed_byte_mask_at_least(v, 0xc2);
		unsigned three = high & signed_byte_mask_at_least(v, 0xe0);
		unsigned four = high & signed_byte_mask_at_least(v, 0xf0);
		unsigned above = high & signed_byte_mask_at_least(v, 0xf5);
		unsigned bit6 = _mm_movemask_epi8(_mm_add_epi8(v, v));
		unsigned trailers = high & ~bit6;
		unsigned expected = (lead << 1) | (three << 2) | (four << 3);
		if (above || high != (trailers | lead) || (expected & 0xffff) != trailers)
			break;

		/* Check the four leads with a restricted second byte. */
		__m128i e0 = _mm_cmpeq_epi8(v, _mm_set1_epi8((char)0xe0));
		__m128i ed = _mm_cmpeq_epi8(v, _mm_set1_epi8((char)0xed));
		__m128i f0 = _mm_cmpeq_epi8(v, _mm_set1_epi8((char)0xf0));
		__m128i f4 = _mm_cmpeq_epi8(v, _mm_set1_epi8((char)0xf4));
		__m128i special3 = _mm_or_si128(e0, ed);
		__m128i special4 = _mm_or_si128(f0, f4);
		if (_mm_movemask_epi8(_mm_or_si128(special3, special4))) {
			__m128i second = _mm_srli_si128(v, 1);
			__m128i gea0 = _mm_cmpgt_epi8(second, _mm_set1_epi8((char)0x9f));
			__m128i ge90 = _mm_cmpgt_epi8(second, _mm_set1_epi8((char)0x8f));
			__m128i bad3 = _mm_or_si128(_mm_andnot_si128(gea0, e0), _mm_and_si128(gea0, ed));
			__m128i bad4 = _mm_or_si128(_mm_andnot_si128(ge90, f0), _mm_and_si128(ge90, f4));
			/* Lane 15 has no second byte in this block. */
			if (_mm_movemask_epi8(_mm_or_si128(bad3, bad4)) & 0x7fff)
				break;
		}

		/* Recheck a character straddling the block in the next pass. */
		unsigned step = 16;
		if (expected >> 16) {
			do {
				step--;
			} while (trailers & (1u << step));
		}
		i += step;
	}
	return i;
}
#endif

static size_t
utf8_validate(const char *str, size_t len)
{
#ifdef __SSE2__
	size_t prefix = len >= 16 ? utf8_validate_blocks(str, len) : 0;
	return prefix + utf8_validate_checked(str + prefix, len - prefix);
#else
	size_t err = 0;

	/* Short inputs fit in one pass through the checked loop. */
	if (len >= 16) {
		err = utf8_validate_fast(str, len - 4);
		str += err;
		len -= err;
	}

	return err + utf8_validate_checked(str, len);
#endif
}

static int
checkutf8_validate(lua_State *l)
{
	size_t len;
	const char *s = luaL_checklstring(l, 1, &len);

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
	size_t len;
	const char *s = luaL_checklstring(l, 1, &len);

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
