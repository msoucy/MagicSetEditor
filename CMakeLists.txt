cmake_minimum_required(VERSION 2.8.12)

project(MagicSetEditor)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
message(STATUS "Module path: ${CMAKE_MODULE_PATH}")

find_package(Boost REQUIRED system)
find_package(wxWidgets REQUIRED)
find_package(HUNSPELL REQUIRED)

include(${wxWidgets_USE_FILE})

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Magic Set Editor")
SET(CPACK_PACKAGE_VENDOR "Matt Soucy")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/index.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CPACK_PACKAGE_EXECUTABLES "magicseteditor")

include(CPack)

if(MSVC)
	add_definitions(
		"/wd4100" # unreferenced formal parameter
		"/wd4355" # 'this' : used in base member initializer list
		"/wd4800" # 'int' : forcing value to bool 'true' or 'false'
		"/wd4675" # resolved overload was found by argument-dependent lookup (occurs in some boost header)
	)
elseif(UNIX)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	if(COMPILER_SUPPORTS_CXX11)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	elseif(COMPILER_SUPPORTS_CXX0X)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	else()
		message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()
	# Force unicode builds - no reason to keep ASCII-only around
	add_definitions("-DUNICODE" "-D_UNICODE")
endif()


IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
	message("debug mode")
	set(EXTRA_FILES
		"src/script/profiler.cpp"
		"src/gui/profiler_window.cpp"
	)
	add_definitions(
		"-DUSE_SCRIPT_PROFILING"
		"-DLOG_UPDATES"
	)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

###############################################################################
# Create and link executable

set(CLI_FILES
	src/cli/cli_main.cpp
	src/cli/cli_main.hpp
	src/cli/text_io_handler.cpp
	src/cli/text_io_handler.hpp
)
source_group(cli FILES ${CLI_FILES})

set(DATA_ACTION_FILES
	src/data/action/generic.hpp
	src/data/action/keyword.cpp
	src/data/action/keyword.hpp
	src/data/action/set.cpp
	src/data/action/set.hpp
	src/data/action/symbol.cpp
	src/data/action/symbol.hpp
	src/data/action/symbol_part.cpp
	src/data/action/symbol_part.hpp
	src/data/action/value.cpp
	src/data/action/value.hpp
)
source_group(data\\action FILES ${DATA_ACTION_FILES})

set(DATA_AUX_FILES
	src/data/add_cards_script.cpp
	src/data/add_cards_script.hpp
	src/data/draw_what.hpp
	src/data/export_template.cpp
	src/data/export_template.hpp
	src/data/filter.hpp
	src/data/font.cpp
	src/data/font.hpp
	src/data/graph_type.hpp
	src/data/installer.cpp
	src/data/installer.hpp
	src/data/keyword.cpp
	src/data/keyword.hpp
	src/data/locale.cpp
	src/data/locale.hpp
	src/data/pack.cpp
	src/data/pack.hpp
	src/data/statistics.cpp
	src/data/statistics.hpp
	src/data/symbol_font.cpp
	src/data/symbol_font.hpp
	src/data/word_list.cpp
	src/data/word_list.hpp
)
source_group(data\\aux FILES ${DATA_AUX_FILES})

set(DATA_BASE_FILES
	src/data/card.cpp
	src/data/card.hpp
	src/data/field.cpp
	src/data/field.hpp
	src/data/game.cpp
	src/data/game.hpp
	src/data/set.cpp
	src/data/set.hpp
	src/data/settings.cpp
	src/data/settings.hpp
	src/data/stylesheet.cpp
	src/data/stylesheet.hpp
	src/data/symbol.cpp
	src/data/symbol.hpp
)
source_group(data\\base FILES ${DATA_BASE_FILES})

set(DATA_FIELD_FILES
	src/data/field/boolean.cpp
	src/data/field/boolean.hpp
	src/data/field/choice.cpp
	src/data/field/choice.hpp
	src/data/field/color.cpp
	src/data/field/color.hpp
	src/data/field/image.cpp
	src/data/field/image.hpp
	src/data/field/information.cpp
	src/data/field/information.hpp
	src/data/field/multiple_choice.cpp
	src/data/field/multiple_choice.hpp
	src/data/field/package_choice.cpp
	src/data/field/package_choice.hpp
	src/data/field/symbol.cpp
	src/data/field/symbol.hpp
	src/data/field/text.cpp
	src/data/field/text.hpp
)
source_group(data\\filed FILES ${DATA_FIELD_FILES})

set(DATA_FORMAT_FILES
	src/data/format/apprentice.cpp
	src/data/format/clipboard.cpp
	src/data/format/clipboard.hpp
	src/data/format/formats.cpp
	src/data/format/formats.hpp
	src/data/format/html.cpp
	src/data/format/image.cpp
	src/data/format/image_to_symbol.cpp
	src/data/format/image_to_symbol.hpp
	src/data/format/mse1.cpp
	src/data/format/mse2.cpp
	src/data/format/mtg_editor.cpp
	src/data/format/mws.cpp
)
source_group(data\\format FILES ${DATA_FORMAT_FILES})

set(GFX_FILES
	src/gfx/bezier.cpp
	src/gfx/bezier.hpp
	src/gfx/blend_image.cpp
	src/gfx/color.cpp
	src/gfx/color.hpp
	src/gfx/combine_image.cpp
	src/gfx/generated_image.cpp
	src/gfx/generated_image.hpp
	src/gfx/gfx.hpp
	src/gfx/image_effects.cpp
	src/gfx/mask_image.cpp
	src/gfx/polynomial.cpp
	src/gfx/polynomial.hpp
	src/gfx/resample_image.cpp
	src/gfx/resample_text.cpp
	src/gfx/rotate_image.cpp
)
source_group(gfx FILES ${GFX_FILES})

set(GUI_CONTROL_FILES
	src/gui/control/card_editor.cpp
	src/gui/control/card_editor.hpp
	src/gui/control/card_list.cpp
	src/gui/control/card_list.hpp
	src/gui/control/card_list_column_select.cpp
	src/gui/control/card_list_column_select.hpp
	src/gui/control/card_viewer.cpp
	src/gui/control/card_viewer.hpp
	src/gui/control/filter_ctrl.cpp
	src/gui/control/filter_ctrl.hpp
	src/gui/control/filtered_card_list.cpp
	src/gui/control/filtered_card_list.hpp
	src/gui/control/gallery_list.cpp
	src/gui/control/gallery_list.hpp
	src/gui/control/graph.cpp
	src/gui/control/graph.hpp
	src/gui/control/image_card_list.cpp
	src/gui/control/image_card_list.hpp
	src/gui/control/item_list.cpp
	src/gui/control/item_list.hpp
	src/gui/control/keyword_list.cpp
	src/gui/control/keyword_list.hpp
	src/gui/control/native_look_editor.cpp
	src/gui/control/native_look_editor.hpp
	src/gui/control/package_list.cpp
	src/gui/control/package_list.hpp
	src/gui/control/select_card_list.cpp
	src/gui/control/select_card_list.hpp
	src/gui/control/text_ctrl.cpp
	src/gui/control/text_ctrl.hpp
	src/gui/control/tree_list.cpp
	src/gui/control/tree_list.hpp
)
source_group(gui\\control FILES ${GUI_CONTROL_FILES})

set(GUI_SET_FILES
	src/gui/set/cards_panel.cpp
	src/gui/set/cards_panel.hpp
	src/gui/set/console_panel.cpp
	src/gui/set/console_panel.hpp
	src/gui/set/keywords_panel.cpp
	src/gui/set/keywords_panel.hpp
	src/gui/set/panel.cpp
	src/gui/set/panel.hpp
	src/gui/set/random_pack_panel.cpp
	src/gui/set/random_pack_panel.hpp
	src/gui/set/set_info_panel.cpp
	src/gui/set/set_info_panel.hpp
	src/gui/set/stats_panel.cpp
	src/gui/set/stats_panel.hpp
	src/gui/set/style_panel.cpp
	src/gui/set/style_panel.hpp
	src/gui/set/window.cpp
	src/gui/set/window.hpp
)
source_group(gui\\set FILES ${GUI_SET_FILES})

set(GUI_SYMBOL_FILES
	src/gui/symbol/basic_shape_editor.cpp
	src/gui/symbol/basic_shape_editor.hpp
	src/gui/symbol/control.cpp
	src/gui/symbol/control.hpp
	src/gui/symbol/editor.cpp
	src/gui/symbol/editor.hpp
	src/gui/symbol/part_list.cpp
	src/gui/symbol/part_list.hpp
	src/gui/symbol/point_editor.cpp
	src/gui/symbol/point_editor.hpp
	src/gui/symbol/select_editor.cpp
	src/gui/symbol/select_editor.hpp
	src/gui/symbol/selection.cpp
	src/gui/symbol/selection.hpp
	src/gui/symbol/symmetry_editor.cpp
	src/gui/symbol/symmetry_editor.hpp
	src/gui/symbol/window.cpp
	src/gui/symbol/window.hpp
)
source_group(gui\\symbol FILES ${GUI_SYMBOL_FILES})

set(GUI_VALUE_FILES
	src/gui/value/choice.cpp
	src/gui/value/choice.hpp
	src/gui/value/color.cpp
	src/gui/value/color.hpp
	src/gui/value/editor.cpp
	src/gui/value/editor.hpp
	src/gui/value/image.cpp
	src/gui/value/image.hpp
	src/gui/value/information.cpp
	src/gui/value/information.hpp
	src/gui/value/multiple_choice.cpp
	src/gui/value/multiple_choice.hpp
	src/gui/value/package_choice.cpp
	src/gui/value/package_choice.hpp
	src/gui/value/symbol.cpp
	src/gui/value/symbol.hpp
	src/gui/value/text.cpp
	src/gui/value/text.hpp
)
source_group(gui\\value FILES ${GUI_VALUE_FILES})

set(GUI_FILES
	src/gui/about_window.cpp
	src/gui/about_window.hpp
	src/gui/auto_replace_window.cpp
	src/gui/auto_replace_window.hpp
	src/gui/card_select_window.cpp
	src/gui/card_select_window.hpp
	src/gui/drop_down_list.cpp
	src/gui/drop_down_list.hpp
	src/gui/html_export_window.cpp
	src/gui/html_export_window.hpp
	src/gui/icon_menu.cpp
	src/gui/icon_menu.hpp
	src/gui/image_slice_window.cpp
	src/gui/image_slice_window.hpp
	src/gui/images_export_window.cpp
	src/gui/images_export_window.hpp
	src/gui/new_window.cpp
	src/gui/new_window.hpp
	src/gui/package_update_list.cpp
	src/gui/package_update_list.hpp
	src/gui/packages_window.cpp
	src/gui/packages_window.hpp
	src/gui/preferences_window.cpp
	src/gui/preferences_window.hpp
	src/gui/print_window.cpp
	src/gui/print_window.hpp
	src/gui/profiler_window.cpp
	src/gui/thumbnail_thread.cpp
	src/gui/thumbnail_thread.hpp
	src/gui/update_checker.cpp
	src/gui/update_checker.hpp
	src/gui/util.cpp
	src/gui/util.hpp
	src/gui/welcome_window.cpp
	src/gui/welcome_window.hpp
)
source_group(gui FILES ${GUI_FILES})

set(RENDER_CARD_FILES
	src/render/card/viewer.cpp
	src/render/card/viewer.hpp
)
source_group(render\\card FILES ${RENDER_CARD_FILES})

set(RENDER_SYMBOL_FILES
	src/render/symbol/filter.cpp
	src/render/symbol/filter.hpp
	src/render/symbol/viewer.cpp
	src/render/symbol/viewer.hpp
)
source_group(render\\symbol FILES ${RENDER_SYMBOL_FILES})

set(RENDER_TEXT_FILES
	src/render/text/compound.cpp
	src/render/text/element.cpp
	src/render/text/element.hpp
	src/render/text/font.cpp
	src/render/text/symbol.cpp
	src/render/text/viewer.cpp
	src/render/text/viewer.hpp
)
source_group(render\\text FILES ${RENDER_TEXT_FILES})

set(RENDER_VALUE_FILES
	src/render/value/choice.cpp
	src/render/value/choice.hpp
	src/render/value/color.cpp
	src/render/value/color.hpp
	src/render/value/image.cpp
	src/render/value/image.hpp
	src/render/value/information.cpp
	src/render/value/information.hpp
	src/render/value/multiple_choice.cpp
	src/render/value/multiple_choice.hpp
	src/render/value/package_choice.cpp
	src/render/value/package_choice.hpp
	src/render/value/symbol.cpp
	src/render/value/symbol.hpp
	src/render/value/text.cpp
	src/render/value/text.hpp
	src/render/value/viewer.cpp
	src/render/value/viewer.hpp
)
source_group(render\\value FILES ${RENDER_VALUE_FILES})

set(SCRIPT_FUNCTIONS_FILES
	src/script/functions/basic.cpp
	src/script/functions/construction.cpp
	src/script/functions/editor.cpp
	src/script/functions/english.cpp
	src/script/functions/export.cpp
	src/script/functions/functions.hpp
	src/script/functions/image.cpp
	src/script/functions/regex.cpp
	src/script/functions/spelling.cpp
	src/script/functions/util.hpp
)
source_group(script\\functions FILES ${SCRIPT_FUNCTIONS_FILES})

set(SCRIPT_FILES
	src/script/context.cpp
	src/script/context.hpp
	src/script/dependency.cpp
	src/script/dependency.hpp
	src/script/image.cpp
	src/script/image.hpp
	src/script/parser.cpp
	src/script/parser.hpp
	src/script/profiler.cpp
	src/script/profiler.hpp
	src/script/script.cpp
	src/script/script.hpp
	src/script/script_manager.cpp
	src/script/script_manager.hpp
	src/script/scriptable.cpp
	src/script/scriptable.hpp
	src/script/to_value.hpp
	src/script/value.cpp
	src/script/value.hpp
)
source_group(script FILES ${SCRIPT_FILES})

set(UTIL_IO_FILES
	src/util/io/get_member.cpp
	src/util/io/get_member.hpp
	src/util/io/package.cpp
	src/util/io/package.hpp
	src/util/io/package_manager.cpp
	src/util/io/package_manager.hpp
	src/util/io/reader.cpp
	src/util/io/reader.hpp
	src/util/io/writer.cpp
	src/util/io/writer.hpp
)
source_group(util\\io FILES ${UTIL_IO_FILES})

set(UTIL_VCS_FILES
	src/util/vcs/subversion.cpp
	src/util/vcs/subversion.hpp
)
source_group(util\\vcs FILES ${UTIL_VCS_FILES})

set(UTIL_FILES
	src/util/action_stack.cpp
	src/util/action_stack.hpp
	src/util/age.cpp
	src/util/age.hpp
	src/util/alignment.cpp
	src/util/alignment.hpp
	src/util/angle.hpp
	src/util/atomic.hpp
	src/util/defaultable.hpp
	src/util/delayed_index_maps.hpp
	src/util/dynamic_arg.hpp
	src/util/error.cpp
	src/util/error.hpp
	src/util/file_utils.cpp
	src/util/file_utils.hpp
	src/util/find_replace.hpp
	src/util/for_each.hpp
	src/util/index_map.hpp
	src/util/locale.hpp
	src/util/order_cache.hpp
	src/util/platform.hpp
	src/util/prec.hpp
	src/util/real_point.hpp
	src/util/reflect.hpp
	src/util/regex.cpp
	src/util/regex.hpp
	src/util/rotation.cpp
	src/util/rotation.hpp
	src/util/paths.cpp
	src/util/paths.hpp
	src/util/smart_ptr.hpp
	src/util/spec_sort.cpp
	src/util/spec_sort.hpp
	src/util/spell_checker.cpp
	src/util/spell_checker.hpp
	src/util/string.cpp
	src/util/string.hpp
	src/util/tagged_string.cpp
	src/util/tagged_string.hpp
	src/util/vcs.cpp
	src/util/vcs.hpp
	src/util/vector2d.hpp
	src/util/version.cpp
	src/util/version.hpp
	src/util/window_id.hpp
)
source_group(util FILES ${UTIL_FILES})

set(FILES
	src/code_template.cpp
	src/code_template.hpp
	src/main.cpp
)
source_group("" FILES ${FILES})

add_executable(magicseteditor
	${CLI_FILES}
	${DATA_ACTION_FILES}
	${DATA_AUX_FILES}
	${DATA_BASE_FILES}
	${DATA_FIELD_FILES}
	${DATA_FORMAT_FILES}
	${GFX_FILES}
	${GUI_CONTROL_FILES}
	${GUI_SET_FILES}
	${GUI_SYMBOL_FILES}
	${GUI_VALUE_FILES}
	${GUI_FILES}
	${RENDER_CARD_FILES}
	${RENDER_SYMBOL_FILES}
	${RENDER_TEXT_FILES}
	${RENDER_VALUE_FILES}
	${SCRIPT_FUNCTIONS_FILES}
	${SCRIPT_FILES}
	${UTIL_IO_FILES}
	${UTIL_VCS_FILES}
	${UTIL_FILES}
	${FILES}
)

target_include_directories(magicseteditor PUBLIC
	src
	${HUNSPELL_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${wxWidgets_INCLUDE_DIRS}
)
target_link_libraries(magicseteditor
	${HUNSPELL_LIBRARIES}
	${Boost_LIBRARIES}
	${wxWidgets_LIBRARIES}
)

###############################################################################
# Install data and executable

set(WX_DATA_DIR share/magicseteditor)

install(
	TARGETS magicseteditor
	DESTINATION .
)

install(
	DIRECTORY data
	DESTINATION ${WX_DATA_DIR}
)

foreach(dr common/ msw/tool msw/icon msw/cursor msw/other/)
install(
	DIRECTORY src/resource/${dr}
	DESTINATION ${WX_DATA_DIR}/resource
)
endforeach()
