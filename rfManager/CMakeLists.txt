cmake_minimum_required(VERSION 3.0)

get_filename_component(THIS_TARGET ${CMAKE_CURRENT_LIST_DIR} NAME)

project(${THIS_TARGET} LANGUAGES CXX)

if(NOT EXISTS ${BASE_DIR})
    set(BASE_DIR ${PROJECT_SOURCE_DIR}/..)
endif()
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/m2m
    ${CMAKE_CURRENT_LIST_DIR}/controller
    ${CMAKE_CURRENT_LIST_DIR}/parser
)
include(${BASE_DIR}/common/cpp/modules.cmake)
setup_target(${THIS_TARGET} ${BASE_DIR})
