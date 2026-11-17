package = "lua-checkutf8"
version = "dev-1"
source = {
	url = "git+https://github.com/ziyao233/lua-checkutf8"
}
description = {
	summary = "Module for validating and repairing UTF-8 sequences",
	homepage = "https://github.com/ziyao233/lua-checkutf8",
	license = "MPL-2.0",
}
dependencies = {
	"lua >= 5.3",
}
build = {
	type = "builtin",
	modules = {
		checkutf8	= "checkutf8.c",
	},
}
