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

## Testing Platforms

- AMD Ryzen 9950X @ 5000MHz - 5700MHz, DDR5 5600MT/s
- Intel Core i5-7300U @ 2700MHz, LPDDR3 1866MT/s
- ARM Cortex-A53 @ 2016MHz on AMLogic A311D, LPDDR4
- ARM Cortex-A73 @ 2208MHz on AMLogic A311D, LPDDR4
- Xuantie C920v1 @ 2000MHz on Sophgo SG2042, DDR4
- Loongson LA464 @ 2500MHz on Loongson-3A5000HV, DDR4 3200MT/s

## Results

### Throughput (MiB/s)

| Text                                     | 9950X | 7300U | A53 | A73 | C920 | LA464 |
|------------------------------------------|-------|-------|-----|-----|------|-------|
| Dream of the Red Chamber                 | 2445  | 1108  | 249 | 412 | 416  | 743   |
| Linux v7.3 Documentation (ja_JP)         | 2267  | 890   | 252 | 413 | 418  | 712   |
| Linux v7.3 Documentation (zh_CN)         | 1914  | 927   | 250 | 412 | 419  | 675   |
| My Blog                                  | 2692  | 948   | 262 | 436 | 444  | 713   |
| Pride and Prejudice                      | 3895  | 1426  | 307 | 580 | 609  | 1026  |
| The Miracles of the Namiya General Store | 2676  | 1213  | 255 | 428 | 432  | 779   |
| Transky                                  | 2533  | 1209  | 253 | 424 | 428  | 774   |

### Throughput per MHz (MiB/s/MHz)

The AMD Ryzen 9950X column uses 5350 MHz, the midpoint of its 5000–5700 MHz
clock range.

| Text                                     | 9950X | 7300U | A53   | A73   | C920  | LA464 |
|------------------------------------------|-------|-------|-------|-------|-------|-------|
| Dream of the Red Chamber                 | 0.457 | 0.410 | 0.123 | 0.187 | 0.208 | 0.297 |
| Linux v7.3 Documentation (ja_JP)         | 0.424 | 0.329 | 0.125 | 0.187 | 0.209 | 0.285 |
| Linux v7.3 Documentation (zh_CN)         | 0.358 | 0.343 | 0.124 | 0.187 | 0.210 | 0.270 |
| My Blog                                  | 0.503 | 0.351 | 0.130 | 0.198 | 0.222 | 0.285 |
| Pride and Prejudice                      | 0.728 | 0.528 | 0.153 | 0.263 | 0.304 | 0.411 |
| The Miracles of the Namiya General Store | 0.500 | 0.449 | 0.127 | 0.194 | 0.216 | 0.312 |
| Transky                                  | 0.474 | 0.448 | 0.125 | 0.192 | 0.214 | 0.310 |
