cmake_minimum_required(VERSION 3.20)
project(Kine LANGUAGES C CXX)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_SHARED_LIBS OFF)
    message(STATUS "Release build: STATIC linking enabled")
else()
    set(BUILD_SHARED_LIBS ON)
    message(STATUS "Debug build: DYNAMIC linking enabled")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# GLFW
set(GLFW_DIR         ${EXTERNAL_DIR}/glfw)
set(GLFW_INCLUDE_DIR ${GLFW_DIR}/include)
set(GLFW_LIB_DIR     ${GLFW_DIR}/lib)

# FreeType
set(FT_DIR         ${EXTERNAL_DIR}/freetype)
set(FT_INCLUDE_DIR ${FT_DIR}/include)
set(FT_LIB_DIR     ${FT_DIR}/lib)

# Glad
set(GLAD_INCLUDE_DIR ${EXTERNAL_DIR}/glad/include)
set(GLAD_SRC ${EXTERNAL_DIR}/glad/src/glad.c)

# ImGui (header-only)
set(IMGUI_INCLUDE_DIR      ${EXTERNAL_DIR}/imgui)
set(RLIMGUI_INCLUDE_DIR    ${EXTERNAL_DIR}/rlimgui)

# Header-only libs
set(GLM_INCLUDE_DIR        ${EXTERNAL_DIR}/glm)
set(ENTT_INCLUDE_DIR       ${EXTERNAL_DIR}/entt)
set(JSON_INCLUDE_DIR       ${EXTERNAL_DIR}/json)
set(STB_INCLUDE_DIR        ${EXTERNAL_DIR}/stb)
set(MINIAUDIO_INCLUDE_DIR  ${EXTERNAL_DIR}/miniaudio)
set(SOL2_INCLUDE_DIR       ${EXTERNAL_DIR}/sol2)
set(LUA_INCLUDE_DIR        ${EXTERNAL_DIR}/lua/include)
set(LUA_LIB_DIR            ${EXTERNAL_DIR}/lua/lib)

include_directories(
    ${GLAD_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${FT_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${RLIMGUI_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${ENTT_INCLUDE_DIR}
    ${JSON_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
    ${MINIAUDIO_INCLUDE_DIR}
    ${SOL2_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}

    ${CMAKE_SOURCE_DIR}/include
)

file(GLOB_RECURSE KINE_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/**/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/**/**/*.cpp"
)

add_library(glad OBJECT ${GLAD_SRC})
set_source_files_properties(${GLAD_SRC} PROPERTIES LANGUAGE C)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
set_target_properties(glad PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
add_library(Kine ${KINE_SOURCES} $<TARGET_OBJECTS:glad>)
set_target_properties(Kine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(Kine PRIVATE
    ${GLFW_INCLUDE_DIR}
    ${ENTT_INCLUDE_DIR}
    ${FT_INCLUDE_DIR}
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_directories(Kine PRIVATE
    ${GLFW_LIB_DIR}
    ${FT_LIB_DIR}
    ${LUA_LIB_DIR}
)

target_sources(Kine PRIVATE $<TARGET_OBJECTS:glad>)

target_link_libraries(Kine PRIVATE
    glad
    glfw
    freetype
    brotlidec
    brotlicommon
    z
    png
    lua
)

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(Kine PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()

# Linux system libs
if(UNIX AND NOT APPLE)
    target_link_libraries(Kine PRIVATE
        pthread
        dl
        X11
    )
endif()


# macOS
if(APPLE)
    target_link_libraries(Kine PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework OpenGL"
        "-framework CoreVideo"
    )
endif()

set_target_properties(Kine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

if(MSVC)
    target_compile_options(Kine PRIVATE
        /W4
        /WX
        /permissive-
        /Zc:__cplusplus
    )
else()
    target_compile_options(Kine PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wshadow
        -Wnon-virtual-dtor
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
endif()


file(GLOB EXAMPLE_DIRS "${CMAKE_SOURCE_DIR}/examples/*")

foreach(example_dir ${EXAMPLE_DIRS})
    if(IS_DIRECTORY ${example_dir})
        get_filename_component(example_name ${example_dir} NAME)

        file(GLOB_RECURSE EXAMPLE_SOURCES
            "${example_dir}/*.cpp"
        )

        if(EXAMPLE_SOURCES)
            add_executable(${example_name} ${EXAMPLE_SOURCES})

            target_link_libraries(${example_name} PRIVATE Kine)

            set_target_properties(${example_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${example_dir}/bin"
            )

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

