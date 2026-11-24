# Benchmark

The [versioned speed corpus](bench/README.md) replays individual agent, tool,
length-sweep and malformed-input calls through both Lua APIs.

## Example Texts

Testing is done with `benchmark.lua` on some real-world texts,

|               Text                            |       Type		| Language			| Size		|
|-----------------------------------------------|-----------------------|-------------------------------|---------------|
| Dream of the Red Chamber                      | Plaintext		| Traditional Chinese		| 2.5MiB	|
| Linux v7.3 Documentation (ja_JP)		| reStructuredText	| Japanese			| 144KiB	|
| Linux v7.3 Documentation (zh_CN)		| reStructuredText	| Simplified Chinese		| 2.3MiB	|
| My Blog					| Markdown		| English/Simplified Chinese	| 124KiB	|
| Pride and Prejudice                           | Plaintext		| English			| 754KiB	|
| The Miracles of the Namiya General Store	| Plaintext		| Japanese			| 637KiB	|
| Transky					| Markdown		| Simplified Chinese		| 6.8MiB	|

For Linux's documentation, my blog, and Transky, the testing text is constructed
by `cat` them together.

`checkutf8.so` is built with Clang 21.1.8 with `-O2` optimization flags.
`benchmark.lua` is invoked with environment variable `ITERATIONS=512`.

## Testing Platforms

- Intel Core i5-7300U @ 2700MHz, LPDDR3 1866MT/s
- ARM Cortex-A53 @ 2016MHz on AMLogic A311D, LPDDR4
- ARM Cortex-A73 @ 2208MHz on AMLogic A311D, LPDDR4
- Xuantie C920v1 @ 2000MHz on Sophgo SG2042, DDR4
- Loongson LA464 @ 2500MHz on Loongson-3A5000HV, DDR4 3200MT/s

## Results

### Throughput (MiB/s)

| Text                                     | 7300U | A53 | A73 | C920 | LA464 |
|------------------------------------------|-------|-----|-----|------|-------|
| Dream of the Red Chamber                 | 1213  | 260 | 453 | 445  | 827   |
| Linux v7.3 Documentation (ja_JP)         | 1070  | 262 | 446 | 427  | 773   |
| Linux v7.3 Documentation (zh_CN)         | 1045  | 257 | 438 | 411  | 732   |
| My Blog                                  | 1111  | 269 | 459 | 419  | 775   |
| Pride and Prejudice                      | 2290  | 308 | 581 | 463  | 1074  |
| The Miracles of the Namiya General Store | 1353  | 268 | 474 | 466  | 874   |
| Transky                                  | 1307  | 267 | 469 | 461  | 860   |

### Throughput per MHz (MiB/s/MHz)

| Text                                     | 7300U | A53   | A73   | C920  | LA464 |
|------------------------------------------|-------|-------|-------|-------|-------|
| Dream of the Red Chamber                 | 0.449 | 0.129 | 0.205 | 0.223 | 0.331 |
| Linux v7.3 Documentation (ja_JP)         | 0.396 | 0.130 | 0.202 | 0.214 | 0.309 |
| Linux v7.3 Documentation (zh_CN)         | 0.387 | 0.127 | 0.198 | 0.205 | 0.293 |
| My Blog                                  | 0.412 | 0.133 | 0.208 | 0.209 | 0.310 |
| Pride and Prejudice                      | 0.848 | 0.153 | 0.263 | 0.232 | 0.430 |
| The Miracles of the Namiya General Store | 0.501 | 0.133 | 0.215 | 0.233 | 0.350 |
| Transky                                  | 0.484 | 0.132 | 0.212 | 0.230 | 0.344 |

### Wide SIMD comparison on Xeon 6966P-C

Lua 5.4.6, Clang 18.1.3, `-O2`, pinned to one core. Both modules were
loaded in the same Lua process. Times are medians over nine interleaved rounds
of calls to `validate` on the same string. The baseline is commit `ab275a6`.
The test machine supports AVX-512BW.

| Lua input | SSE2 (GiB/s) | Runtime dispatch (GiB/s) |
| --- | ---: | ---: |
| `("文"):rep(64)` | 2.94 | 5.50 |
| `("文"):rep(512)` | 3.89 | 14.92 |
| `("文"):rep(65536)` | 4.22 | 22.42 |
| `("🌛😃🚀🌃"):rep(15000)` | 3.80 | 22.31 |
| `("function hello() return true end\n"):rep(3000)` | 54.68 | 77.72 |

Scanning one 576 MiB string, `("中文段落"):rep(48 * 1024^2)`, yielded
11.50 GiB/s with runtime dispatch (median of seven rounds).

### Aligned loads and the AVX-512VBMI path

Comparison with commit `71eee70` on the same Xeon, Lua 5.4.6 and Clang 18
at `-O2`. Both modules ran in one process pinned to core 4. Each value is the
median of nine rounds of `validate` calls through Lua. Small buffers were
reused across calls. All figures below are GiB/s.

| Input | Previous | New |
| --- | ---: | ---: |
| `("微风吹过湖面，我们沿着山路慢慢往上走。"):rep(3300)` | 22.17 | 101.36 |
| `(("中"):rep(315) .. "\n"):rep(200)` | 22.46 | 35.26 |
| `("今天检查了 API 请求，发现 worker 在读完 JSON 后多等了 20 ms。"):rep(1500)` | 22.36 | 26.33 |
| `("function hello() return true end\n"):rep(3000)` | 73.98 | 122.57 |
| `("中文段落"):rep(48 * 1024^2)` (576 MiB) | 10.72 | 14.18 |
| `("Aλ中🌙"):rep(math.floor(576 * 1024^2 / 10))` (about 576 MiB) | 8.09 | 8.90 |

For 192 bytes of Chinese, the median call time was 31.99 ns before and 32.14 ns
after. With AVX2 forced on this Xeon, the long Chinese sample went from 17.37
to 17.82 GiB/s and the ASCII sample from 66.15 to 98.29 GiB/s.

### Short inputs with AVX-512VBMI

Lua 5.4.6 on Xeon 6966P-C, core 4, Clang 18 at `-O2`. The baseline is
`a4769f5`. Each group replays the fixed samples in corpus order through
`validate`, with both modules loaded and nine interleaved timing rounds.
Times are median ns per Lua call including loop overhead.

| Corpus group | Previous | Short-block path |
| --- | ---: | ---: |
| red-chamber-4cp | 24.66 | 19.22 |
| red-chamber-16cp | 32.20 | 19.24 |
| red-chamber-64cp | 34.58 | 25.04 |
| red-chamber-256cp | 60.42 | 60.14 |
| agent-session | 30.78 | 22.58 |
| text-deltas | 22.06 | 22.85 |
| raw-byte-chunks | 23.48 | 20.86 |
| tool-log-replay | 27.59 | 22.62 |

The dedicated path handles 5 through 511 bytes on CPUs supporting AVX-512BW
and VBMI. Whole-book validation took 134.8 µs before and 134.0 µs after.
