cmake_minimum_required(VERSION 3.10)

function(setup_target TAR BASE_DIR)
    set(COMMON_DIR ${BASE_DIR}/common/cpp)

    file(GLOB common_sources
        "${COMMON_DIR}/*/*.h"
        "${COMMON_DIR}/*/*.cpp"
    )

    file(GLOB local_sources
        "${BASE_DIR}/${TAR}/*/*.h"
        "${BASE_DIR}/${TAR}/*/*.cpp"
    )

    add_executable(${TAR}
        ${TAR}.cpp
        ${common_sources}
        ${local_sources}
    )

    target_include_directories(${TAR} PRIVATE ${BASE_DIR}/${TAR})
    target_include_directories(${TAR} PRIVATE ${COMMON_DIR})

    target_compile_features(${TAR} PRIVATE cxx_std_17)

    add_definitions(-DPROCESS_NAME="${TAR}")
    target_compile_options(${TAR} PUBLIC -Werror -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic -Wold-style-cast
        -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wmisleading-indentation
        -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion
        -Wformat=2 -Weffc++ -g -pipe -flto -Xlinker -Map=${TAR}.map)

    target_link_libraries(${TAR} PUBLIC dl zmq pthread)

    install(TARGETS ${THIS_TARGET} DESTINATION ${BASE_DIR}/build/bin/)
endfunction()
