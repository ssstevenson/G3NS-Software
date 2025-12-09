    cmake_minimum_required(VERSION 3.10)

    get_filename_component(THIS_TARGET ${CMAKE_CURRENT_LIST_DIR} NAME)

    project(${THIS_TARGET} LANGUAGES CXX)

    if(NOT EXISTS ${BASE_DIR})
        set(BASE_DIR ${PROJECT_SOURCE_DIR}/..)
    endif()

    # Define the source directory for fpgaPeekPoke
    set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    # Find all source files for this target
    file(GLOB sources
            "${SRC_DIR}/*.h"
            "${SRC_DIR}/*.cpp"
    )

    # Add the executable target
    add_executable(${THIS_TARGET} ${sources})

    # Add include directories
    target_include_directories(${THIS_TARGET} PRIVATE ${SRC_DIR})
    # Add common/cpp to include directories so headers like <FpgaIO/fpgaIO.h> can be found
    target_include_directories(${THIS_TARGET} PUBLIC ${BASE_DIR}/common/cpp)

    # Set C++ standard and compile options (example from moduleManager/CMakeLists.txt)
    target_compile_features(${THIS_TARGET} PRIVATE cxx_std_17)
    add_definitions(-DPROCESS_NAME="${THIS_TARGET}")
    target_compile_options(${THIS_TARGET} PUBLIC -Werror -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic -Wold-style-cast
            -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wmisleading-indentation
            -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion
            -Wformat=2 -Weffc++ -g -pipe -flto -Xlinker -Map=${THIS_TARGET}.map)

    # Link against the common_lib which contains FpgaIO definitions
    target_link_libraries(${THIS_TARGET} PUBLIC common_lib -ldl -lpthread rt)

    # Install the target
    install(TARGETS ${THIS_TARGET} DESTINATION ${BASE_DIR}/build/bin/)
