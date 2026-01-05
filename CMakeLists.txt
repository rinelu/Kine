cmake_minimum_required(VERSION 3.20)
project(Kine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTERNAL ${CMAKE_SOURCE_DIR}/external)

# Glad
add_library(glad OBJECT ${EXTERNAL}/glad/src/glad.c)
target_include_directories(glad PUBLIC ${EXTERNAL}/glad/include)

# Zlib
file(GLOB ZLIB_SRC ${EXTERNAL}/zlib/*.c)
add_library(zlib STATIC ${ZLIB_SRC})
target_include_directories(zlib PRIVATE ${EXTERNAL}/zlib)

# libpng
file(GLOB PNG_SRC ${EXTERNAL}/libpng/*.c)
add_library(png STATIC ${PNG_SRC})
target_include_directories(png PUBLIC ${EXTERNAL}/libpng)
target_link_libraries(png PUBLIC zlib)

# Brotli
file(GLOB_RECURSE BROTLI_COMMON_SRC ${EXTERNAL}/brotli/common/*.c)
file(GLOB_RECURSE BROTLI_DEC_SRC ${EXTERNAL}/brotli/dec/*.c)

add_library(brotlicommon STATIC ${BROTLI_COMMON_SRC})
add_library(brotlidec STATIC ${BROTLI_DEC_SRC})

target_include_directories(brotlicommon PUBLIC ${EXTERNAL}/brotli/include)
target_include_directories(brotlidec PUBLIC ${EXTERNAL}/brotli/include)
target_link_libraries(brotlidec PUBLIC brotlicommon)

# ImGui
file(GLOB IMGUI_SRC ${EXTERNAL}/imgui/imgui*.cpp)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${EXTERNAL}/imgui)

# FreeType
set(FREETYPE_DIR ${EXTERNAL}/freetype)
add_library(freetype STATIC)
target_include_directories(freetype PUBLIC
    ${FREETYPE_DIR}/include
    ${EXTERNAL}/zlib)
target_compile_definitions(freetype PRIVATE
    FT2_BUILD_LIBRARY
    FT_CONFIG_OPTION_SYSTEM_ZLIB)

set(FT_BASE
    ${FREETYPE_DIR}/src/base/ftbase.c
    ${FREETYPE_DIR}/src/base/ftinit.c
    ${FREETYPE_DIR}/src/base/ftsystem.c
    ${FREETYPE_DIR}/src/base/ftbitmap.c
    ${FREETYPE_DIR}/src/base/ftdebug.c
    ${FREETYPE_DIR}/src/base/ftmm.c
)
set(FT_COMP
    ${FREETYPE_DIR}/src/gzip/ftgzip.c
    ${FREETYPE_DIR}/src/lzw/ftlzw.c)
set(FT_RENDER
    ${FREETYPE_DIR}/src/raster/raster.c
    ${FREETYPE_DIR}/src/smooth/smooth.c
    ${FREETYPE_DIR}/src/sdf/sdf.c
    ${FREETYPE_DIR}/src/svg/svg.c
)
set(FT_DRIVERS
    ${FREETYPE_DIR}/src/autofit/autofit.c
    ${FREETYPE_DIR}/src/truetype/truetype.c
    ${FREETYPE_DIR}/src/type1/type1.c
    ${FREETYPE_DIR}/src/type42/type42.c
    ${FREETYPE_DIR}/src/cff/cff.c
    ${FREETYPE_DIR}/src/cid/type1cid.c
    ${FREETYPE_DIR}/src/pfr/pfr.c
    ${FREETYPE_DIR}/src/winfonts/winfnt.c
    ${FREETYPE_DIR}/src/pcf/pcf.c
    ${FREETYPE_DIR}/src/bdf/bdf.c
)
set(FT_HELPERS
    ${FREETYPE_DIR}/src/psaux/psaux.c
    ${FREETYPE_DIR}/src/psnames/psnames.c
    ${FREETYPE_DIR}/src/pshinter/pshinter.c
    ${FREETYPE_DIR}/src/sfnt/sfnt.c
)
set(FT_MODULES
    ${FREETYPE_DIR}/src/autofit/autofit.c
    ${FREETYPE_DIR}/src/raster/raster.c
    ${FREETYPE_DIR}/src/smooth/smooth.c
    ${FREETYPE_DIR}/src/truetype/truetype.c
    ${FREETYPE_DIR}/src/type1/type1.c
    ${FREETYPE_DIR}/src/cff/cff.c
    ${FREETYPE_DIR}/src/sfnt/sfnt.c
    ${FREETYPE_DIR}/src/psaux/psaux.c
    ${FREETYPE_DIR}/src/psnames/psnames.c
)
set(FT_OPTIONAL
    ${FREETYPE_DIR}/src/sfnt/sfwoff.c
    ${FREETYPE_DIR}/src/sfnt/sfwoff2.c
    ${FREETYPE_DIR}/src/sfnt/woff2tags.c
    ${FREETYPE_DIR}/src/sfnt/pngshim.c
)

target_sources(freetype PRIVATE
    ${FT_BASE}
    ${FT_COMP}
    ${FT_RENDER}
    ${FT_DRIVERS}
    ${FT_HELPERS}
    ${FT_OPTIONAL}
)

target_link_libraries(freetype PUBLIC
    png
    zlib
    brotlidec
    brotlicommon
)

# GLFW
add_subdirectory(${EXTERNAL}/glfw)

# Kine
file(GLOB_RECURSE KINE_SRC ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_library(Kine STATIC
    ${KINE_SRC}
    $<TARGET_OBJECTS:glad>
)

target_include_directories(Kine PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${EXTERNAL}/glm
    ${EXTERNAL}/entt
    ${EXTERNAL}/stb
    ${EXTERNAL}/miniaudio
    ${EXTERNAL}/freetype/include
    ${EXTERNAL}/glad/include
)

# Link everything
target_link_libraries(Kine PRIVATE
    imgui
    freetype
    glfw
    # brotlidec
    # brotlicommon
    png
)

# Platform libs
if(UNIX AND NOT APPLE)
    target_link_libraries(Kine PRIVATE
        pthread
        dl
        X11
    )
endif()

if(APPLE)
    target_link_libraries(Kine PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework OpenGL"
        "-framework CoreVideo"
    )
endif()

if(MSVC)
    target_compile_options(Kine PRIVATE /W4 /WX)
else()
    target_compile_options(Kine PRIVATE
        -Wall -Wextra -Werror
        -Wshadow
        -Wnon-virtual-dtor
        -Wcast-align
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
endif()

file(GLOB EXAMPLE_DIRS "${CMAKE_SOURCE_DIR}/examples/*")

foreach(example_dir ${EXAMPLE_DIRS})
    if(IS_DIRECTORY ${example_dir})
        get_filename_component(example_name ${example_dir} NAME)

        file(GLOB_RECURSE EXAMPLE_SOURCES "${example_dir}/*.cpp")

        if(EXAMPLE_SOURCES)
            add_executable(${example_name} ${EXAMPLE_SOURCES})
            target_link_libraries(${example_name} PRIVATE Kine)
            set_target_properties(${example_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${example_dir}/bin")

            if(EXISTS "${example_dir}/assets")
                add_custom_command(
                    TARGET ${example_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${example_dir}/assets"
                        "${example_dir}/bin/assets"
                )
            endif()
        endif()
    endif()
endforeach()

