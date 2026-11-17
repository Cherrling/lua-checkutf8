local checkutf8			= require "checkutf8";

local nIterations = math.tointeger(os.getenv("ITERATIONS") or 128);

local function
doBench(file)
	local s = io.open(file, "r"):read("a");
	local validate = checkutf8.validate;

	print(("%s: Validation result: %q"):format(file, validate(s)));

	for i = 1, 8 do
		validate(s);
	end

	local t1 = os.clock();

	for i = 1, nIterations do
		validate(s);
	end

	local t2 = os.clock();

	local size = #s * nIterations / 1024 / 1024;
	print(("%s: Validated %.2f MiB data in %.2fs, %.2f MiB/s"):
	      format(file, size, t2 - t1, size / (t2 - t1)));
end

for _, f in ipairs(arg) do
	doBench(f);
end
