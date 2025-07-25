project('ps4', 'c', 'cpp', 'nasm', version: '1.0.0')
warning_level = 3
cpp_std = 'c++20'
debug = true
optimization = 3
b_lto = true
b_pgo = 'use'

cpp = meson.get_compiler('cpp')

add_project_arguments(
	cpp.get_supported_arguments(
		'-D_WIN32_WINNT=_WIN32_WINNT_WIN10',
		'-std=c++20',
		'-masm=intel',
		'-Werror',
	),
	language: 'cpp',
)

add_project_link_arguments(
	cpp.get_supported_arguments(
		'-static',
		'-s',
		'-std=c++20',
		'-ld3dcompiler',
	),
	language: 'cpp',
)

minhook = subproject('minhook')
tomlc99 = subproject('tomlc99')
libpng = subproject('libpng', default_options: ['default_library=static'])

library(
	'ps4',
	include_directories: [
		'src',
	],
	cpp_pch : 'src/helpers.h',
	dependencies : [
		minhook.get_variable('minhook_dep'),
		tomlc99.get_variable('tomlc99_dep'),
		libpng.get_variable('libpng_dep'),
	],
	sources : [
		'src/mod.cpp',
		'src/SigScan.cpp',
		'src/diva.cpp',
		'src/hooks.asm',
		'src/menus/exit.cpp',
		'src/menus/gallery.cpp',
		'src/menus/options.cpp',
		'src/menus/pause.cpp',
		'src/menus/pvSel.cpp',
		'src/menus/customize.cpp',
		'src/menus/customize.asm',
		'src/menus/result.cpp',
		'src/menus/decoration.cpp',
		'src/menus/pvWatch.cpp',
		'src/menus/pvGame.cpp',
		'src/menus/pvGame.asm',
		'src/menus/genericDialog.cpp',
		'src/menus/genericDialog.asm',
		'src/menus/commonUi.cpp',
		'src/menus/commonMenu.cpp',
	],
	name_prefix: ''
)
