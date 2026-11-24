// SPDX-License-Identifier: MPL-2.0
/*
 * checkutf8 - Module for validating and repairing UTF-8
 * Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

/*
 * Some futher optimizations
 * - Add a fast path to process likely ASCII stuff in batch, utilizing the full
 *   machine register (4/8 bytes).
 */

// Trailing bytes are in range [0x80, 0xbf], or [0b1000, 0b1011], i.e. their
// most significant two bits must be 0b10.
#define valid_trailer(c)	(((c) & 0xc0) == 0x80)

static size_t
UTF8_VALIDATE(const char *str, size_t len)
{
	const uint8_t *p = (const uint8_t *)str;
	size_t i = 0;

	while (i < len) {
		if (p[i] <= 0x7f) {
			/* [00, 7f] */
			i++;
		} else if (p[i] >= 0xc2 && p[i] <= 0xdf) {
			/* [c2 - df] [80 - bf] */
#ifdef BOUNDARY_CHECK
			if (len - i < 2)
				return i;
#endif

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
#ifdef BOUNDARY_CHECK
			if (len - i < 3)
				return i;
#endif

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
#ifdef BOUNDARY_CHECK
			if (len - i < 4)
				return i;
#endif

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

#undef valid_trailer
