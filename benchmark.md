# Benchmark

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
