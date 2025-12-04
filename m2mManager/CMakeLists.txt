cmake_minimum_required(VERSION 3.10)

get_filename_component(THIS_TARGET ${CMAKE_CURRENT_LIST_DIR} NAME)

project(${THIS_TARGET} LANGUAGES CXX)

if(NOT EXISTS ${BASE_DIR})
    set(BASE_DIR ${PROJECT_SOURCE_DIR}/..)
endif()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/common
    ${CMAKE_CURRENT_LIST_DIR}/ipc
    ${CMAKE_CURRENT_LIST_DIR}/network
)
include(${BASE_DIR}/common/cpp/modules.cmake)
setup_target(${THIS_TARGET} ${BASE_DIR})
