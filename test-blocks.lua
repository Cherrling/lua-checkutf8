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
