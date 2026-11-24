// SPDX-License-Identifier: MPL-2.0
/* Include once per vector width. The compiler emits both targets in one
 * module without requiring AVX instructions in the baseline build. */
#if VECTOR_BYTES == 32
#define TARGET __attribute__((target("avx2")))
#define NAME(f) f##_avx2
#define V __m256i
#define OP(f) _mm256_##f
#define LOAD(p) _mm256_loadu_si256((const __m256i_u *)(p))
#define TABLE(p) _mm256_broadcastsi128_si256(load16((const char *)(p)))
#define HIGH(v) _mm256_movemask_epi8(v)
#define ANY(v) (!_mm256_testz_si256(v, v))
#else
#define TARGET __attribute__((target("avx512f,avx512bw")))
#define NAME(f) f##_avx512
#define V __m512i
#define OP(f) _mm512_##f
#define LOAD(p) _mm512_loadu_si512(p)
#define TABLE(p) _mm512_broadcast_i32x4(load16((const char *)(p)))
#define HIGH(v) _mm512_movepi8_mask(v)
#define ANY(v) _mm512_test_epi64_mask(v, v)
#endif

TARGET static inline V
NAME(pair_errors)(const char *p, V current)
{
	V prev1 = LOAD(p - 1);
	V prev2 = LOAD(p - 2);
	V prev3 = LOAD(p - 3);
	V nibble = OP(set1_epi8)(15);
	V pairs = OP(shuffle_epi8)(TABLE(prev_high), OP(srli_epi16)(prev1, 4) & nibble)
	        & OP(shuffle_epi8)(TABLE(prev_low), prev1 & nibble)
	        & OP(shuffle_epi8)(TABLE(curr_high), OP(srli_epi16)(current, 4) & nibble);

	/* The sign bits indicate a three- or four-byte lead at p-2, or
	 * a four-byte lead at p-3. Both require two adjacent trailers. */
	V third = OP(subs_epu8)(prev2, OP(set1_epi8)(0x60));
	V fourth = OP(subs_epu8)(prev3, OP(set1_epi8)(0x70));
	return pairs ^ ((third | fourth) & OP(set1_epi8)((char)0x80));
}

TARGET static size_t
NAME(utf8_validate)(const char *str, size_t len, size_t i)
{
	while (len - i >= 4 * VECTOR_BYTES) {
		const char *p = str + i;
		V a = LOAD(p);
		V b = LOAD(p + VECTOR_BYTES);
		V c = LOAD(p + 2 * VECTOR_BYTES);
		V d = LOAD(p + 3 * VECTOR_BYTES);
		if (HIGH((a | b) | (c | d)) || (unsigned char)p[-1] >= 128) {
			V ab = NAME(pair_errors)(p, a) | NAME(pair_errors)(p + VECTOR_BYTES, b);
			V cd = NAME(pair_errors)(p + 2 * VECTOR_BYTES, c) | NAME(pair_errors)(p + 3 * VECTOR_BYTES, d);
			V errors = ab | cd;
			if (ANY(errors))
				break;  /* Locate the failing vector below. */
		}
		i += 4 * VECTOR_BYTES;
	}
	while (len - i >= VECTOR_BYTES) {
		V errors = NAME(pair_errors)(str + i, LOAD(str + i));
		if (ANY(errors))
			break;
		i += VECTOR_BYTES;
	}

	/* Recheck the last character, whose trailing bytes may be in the tail. */
	if ((unsigned char)str[i - 1] >= 128) {
		i--;
		while (((unsigned char)str[i] & 0xc0) == 0x80)
			i--;
	}
	return i;
}
#undef TARGET
#undef NAME
#undef V
#undef OP
#undef LOAD
#undef TABLE
#undef HIGH
#undef ANY
