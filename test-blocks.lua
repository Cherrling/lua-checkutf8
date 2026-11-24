-- Exercise UTF-8 sequences on both sides of the vector block boundaries.
local checkutf8 = require "checkutf8"

local function check(input)
	local count, first = utf8.len(input)
	local valid, offset = checkutf8.validate(input)
	assert(valid == (count ~= nil) and offset == first)

	local parts, start = {}, 1
	while first do
		parts[#parts + 1] = input:sub(start, first - 1) .. "?"
		start = first + 1
		count, first = utf8.len(input, start)
	end
	assert(checkutf8.repair(input, "?") == table.concat(parts) .. input:sub(start))
end

for prefix = 0, 80 do
	local pad = ("a"):rep(prefix)
	for _, lead in ipairs({0x80, 0xc0, 0xc1, 0xc2, 0xdf, 0xe0, 0xe1, 0xed,
	                       0xef, 0xf0, 0xf1, 0xf4, 0xf5, 0xff}) do
		for _, second in ipairs({0, 0x7f, 0x80, 0x8f, 0x90, 0x9f, 0xa0, 0xbf, 0xc0}) do
			local seq = string.char(lead, second, 0x80, 0xbf)
			for n = 1, #seq do
				local s = pad .. seq:sub(1, n)
				check(s)
				check(s .. "中🙂" .. pad)
			end
		end
	end
end

-- Move a sequence through every lane of a four-vector group. Check both
-- a truncated final character and an error followed by another full group.
for offset = 0, 255 do
	local pad = ("a中"):rep(128) .. ("x"):rep(offset)
	for _, seq in ipairs({
		"\xc0\x80", "\xc1\xbf", "\xc2\x80", "\xe0\x9f\xbf", "\xe0\xa0\x80",
		"\xed\x9f\xbf", "\xed\xa0\x80", "\xf0\x8f\xbf\xbf", "\xf0\x90\x80\x80",
		"\xf4\x8f\xbf\xbf", "\xf4\x90\x80\x80", "\xf5\x80\x80\x80", "\xff",
	}) do
		for n = 1, #seq do
			local s = pad .. seq:sub(1, n)
			check(s)
			check(s .. ("a"):rep(256) .. "\xff" .. pad)
		end
	end
end
