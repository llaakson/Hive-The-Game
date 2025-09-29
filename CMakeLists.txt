cmake_minimum_required(VERSION 3.16)
project(HiveTheGame LANGUAGES CXX)

# ---- Build settings ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optionally build with static SFML on Windows if you have it installed that way.
option(SFML_STATIC "Link SFML statically (Windows usually)" OFF)
if (SFML_STATIC)
    add_definitions(-DSFML_STATIC)
endif()

# ---- Sources ----
add_executable(hive
    main.cpp
    Hive.cpp
    Hive.hpp
)

# ---- SFML ----
# Try to find SFML 2.5+; adjust components if your code doesn't use some of them.
find_package(SFML 2.5 REQUIRED COMPONENTS system window graphics audio)

target_link_libraries(hive PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# If you're using vcpkg or a custom SFML install, you might need to hint SFML_DIR, e.g.:
# cmake -S . -B build -DSFML_DIR=/path/to/SFML/lib/cmake/SFML

# ---- Platform niceties ----
if (MSVC)
    # Use more useful warnings on MSVC
    target_compile_options(hive PRIVATE /W4 /permissive-)
else()
    target_compile_options(hive PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Put runtime next to the source dir for easier testing (on single-config generators like Ninja/Unix Makefiles)
set_target_properties(hive PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Optionally copy runtime assets (fonts, textures, etc.) next to the binary after build.
# Add any asset files here if you want them automatically copied.
set(HIVE_ASSETS
    arial.ttf
    tileset.png
)
foreach(asset IN LISTS HIVE_ASSETS)
    if (EXISTS "${CMAKE_SOURCE_DIR}/${asset}")
        add_custom_command(TARGET hive POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_SOURCE_DIR}/${asset}"
                    "$<TARGET_FILE_DIR:hive>/${asset}"
        )
    endif()
endforeach()
