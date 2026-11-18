# lua-checkutf8

A simple Lua module for validating and repairing UTF-8 sequences.

## Installation

This module consists of only one C file, `checkutf8.c`, which could either
be built manually with a C compiler, or through luarocks.

lua-checkutf8 requires Lua 5.3 or later, and is only tested with Lua 5.4 and Lua
5.5.

## API

### Validate

```
valid, errorOffset = checkutf8.validate(str)
```

Check for illegal UTF-8 sequences in string `str`. When encountering an invalid
sequence, the function returns `false` and its one-based start position indexed
by bytes; `true` is returned otherwise.

Embedded zeros are allowed in `str`.

### Repair

```
utf8seq = checkutf8.repair(str, [replacement]);
```

Replace each byte of illegal UTF-8 sequences in string `str` with string
`replacement` to generate a valid UTF-8 sequence. `replacement` defaults to
the Unicode replacement character (`U+FFFD`) encoded with UTF-8, i.e.,
`\xef\xbf\xbd`.

It's the caller's responsibility to ensure `replacement` is a valid UTF-8
sequence. Embedded zeros are allowed in both `str` and `replacement`.

Since this function performs replacement for each byte of the illegal sequences
instead of their maximal subparts, it isn't compliant with WHATWG Encoding
Standard, but the Unicode Standard doesn't require such behavior.

## Copyright and License

This project is written by `Yao Zi <me@ziyao.cc>`, and released under Mozilla
Public License version 2.0. Refer to `LICENSE` for details.
