# =============================================================================
# Root CMakeLists.txt
# Place this in your project's root directory (e.g., Software/)
# =============================================================================
cmake_minimum_required(VERSION 3.10)

# --- Project Definition ---
# Set a name for the entire project solution.
project(MultiTargetProject CXX)

# --- Global Settings ---
# Set the C++ standard for the entire project. Targets can override this if needed.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Output Paths ---
# This will place all executables in 'build/bin' and libraries in 'build/lib'
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# --- Common Library Definition ---
# This section creates a single library for all the code in the 'common/cpp' directory.
# All other targets will link against this to use the shared code.

# Define the path to the common code.
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/cpp)

# Use file(GLOB_RECURSE) to find all .cpp and .h files in the common directory.
# This makes it easy to add new files to 'common' without updating the CMakeLists.
file(GLOB_RECURSE COMMON_SOURCES
    "${COMMON_DIR}/*.cpp"
    "${COMMON_DIR}/*.h"
)

# Create a static library called 'common_lib' from the collected source files.
# Using a library is much cleaner than adding source files to each executable directly.
add_library(common_lib STATIC ${COMMON_SOURCES})

# Make the 'common/cpp' directory available as an include path for any target
# that links against 'common_lib'. This is the modern CMake way to handle includes.
target_include_directories(common_lib PUBLIC ${COMMON_DIR})

# --- Add Executable Targets ---
# Use add_subdirectory() for each of your executable targets.
# CMake will look for a CMakeLists.txt inside each of these directories and process it.
#
# To add a new target, just create its folder and add a new line here.
add_subdirectory(cduClient)
add_subdirectory(environmentManager)
add_subdirectory(fpgaHardwareInterface)
add_subdirectory(fpgaPeekPoke)
add_subdirectory(m2mManager)
add_subdirectory(moduleManager)
add_subdirectory(mqttInterface)
add_subdirectory(rfManager)
add_subdirectory(serialHardwareInterface)
add_subdirectory(switchManager)
# add_subdirectory(my_new_target_folder)
