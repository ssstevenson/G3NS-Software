cmake_minimum_required(VERSION 3.10)

get_filename_component(THIS_TARGET ${CMAKE_CURRENT_LIST_DIR} NAME)

project(${THIS_TARGET} LANGUAGES CXX)

if(NOT EXISTS ${BASE_DIR})
    set(BASE_DIR ${PROJECT_SOURCE_DIR}/..)
endif()

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

file(GLOB sources
    "${SRC_DIR}/*.h"
    "${SRC_DIR}/*.cpp"
)

add_executable(${THIS_TARGET} ${sources})

target_include_directories(${THIS_TARGET} PRIVATE ${SRC_DIR})

target_compile_features(${THIS_TARGET} PRIVATE cxx_std_17)

add_definitions(-DPROCESS_NAME="${THIS_TARGET}")
target_compile_options(${THIS_TARGET} PUBLIC -Werror -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic -Wold-style-cast
    -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wmisleading-indentation
    -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion
    -Wformat=2 -Weffc++ -g -pipe -flto -Xlinker -Map=${THIS_TARGET}.map)

target_link_libraries(${THIS_TARGET} PUBLIC -ldl -lpthread)
install(TARGETS ${THIS_TARGET} DESTINATION ${BASE_DIR}/build/bin/)
