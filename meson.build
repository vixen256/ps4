project('ps4', 'c', 'cpp', 'nasm', version: '1.0.0')
warning_level = 3
cpp_std = 'c++20'
optimization = 3
b_lto = true
b_pgo = 'use'

cpp = meson.get_compiler('cpp')

add_project_arguments(
	cpp.get_supported_arguments(
		'-D_WIN32_WINNT=_WIN32_WINNT_WIN10',
		'-std=c++20',
		'-masm=intel',
		'-O3',
	),
	language: 'cpp',
)

add_project_link_arguments(
	cpp.get_supported_arguments(
		'-static',
		'-s',
		'-std=c++20',
		'-ld3dcompiler',
		'-O3',
	),
	language: 'cpp',
)

minhook = subproject('minhook')
tomlc99 = subproject('tomlc99')
libpng = subproject('libpng', default_options: ['default_library=static'])
steam = cpp.find_library('steam_api64', dirs: join_paths(meson.current_source_dir(), 'steam/lib'))

library(
	'ps4',
	include_directories: [
		'src',
		'steam',
	],
	cpp_pch : 'src/helpers.h',
	dependencies : [
		minhook.get_variable('minhook_dep'),
		tomlc99.get_variable('tomlc99_dep'),
		libpng.get_variable('libpng_dep'),
		steam
	],
	sources : [
		'src/mod.cpp',
		'src/SigScan.cpp',
		'src/diva.cpp',
		'src/steam.cpp',
		'src/hooks.asm',
		'src/menus/exit.cpp',
		'src/menus/gallery.cpp',
		'src/menus/options.cpp',
		'src/menus/pause.cpp',
		'src/menus/pvSel.cpp',
		'src/menus/pvSel.asm',
		'src/menus/customize.cpp',
		'src/menus/customize.asm',
		'src/menus/result.cpp',
		'src/menus/result.asm',
		'src/menus/decoration.cpp',
		'src/menus/pvWatch.cpp',
		'src/menus/pvGame.cpp',
		'src/menus/genericDialog.cpp',
		'src/menus/genericDialog.asm',
		'src/menus/commonUi.cpp',
		'src/menus/commonMenu.cpp',
		'src/menus/leaderboard.cpp',
		'src/menus/gamma.cpp',
	],
	name_prefix: ''
)
