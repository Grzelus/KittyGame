cmake_minimum_required(VERSION 3.16)
project(Kitty)

set(CMAKE_CXX_STANDARD 17)

# --- Download SFML ---
include(FetchContent)
FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.1
)
FetchContent_MakeAvailable(sfml)

# --- Define Executable ---
# Make sure "KittyGame" matches your target name
add_executable(KittyGame KittyGame/main.cpp)

# --- Link SFML ---
# This command automatically fixes the "file not found" error
target_link_libraries(KittyGame PRIVATE sfml-graphics sfml-window sfml-system)

# Define where your assets are currently located
# CHANGE "KittyGame/assets" to the actual path relative to CMakeLists.txt
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/KittyGame/assets")
set(FONTS_DIR "${CMAKE_SOURCE_DIR}/KittyGame/arial.ttf")

# Copy the assets folder to the build directory
file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${FONTS_DIR} DESTINATION ${CMAKE_BINARY_DIR})