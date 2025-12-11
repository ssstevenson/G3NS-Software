cmake_minimum_required(VERSION 3.10)

function(setup_target TAR SOFTWARE_DIR)
    set(COMMON_DIR ${SOFTWARE_DIR}/common/cpp)

    file(GLOB common_sources
        "${COMMON_DIR}/*/*.h"
        "${COMMON_DIR}/*/*.cpp"
    )

    file(GLOB local_sources
        "${SOFTWARE_DIR}/${TAR}/*/*.h"
        "${SOFTWARE_DIR}/${TAR}/*/*.cpp"
        "${SOFTWARE_DIR}/${TAR}/*.h"
        "${SOFTWARE_DIR}/${TAR}/*.cpp"
    )

    add_executable(${TAR}
        ${SOFTWARE_DIR}/${TAR}/${TAR}.cpp
        ${common_sources}
        ${local_sources}
    )

    target_include_directories(${TAR} PRIVATE
        ${SOFTWARE_DIR}/${TAR}
        ${COMMON_DIR}
    )

    target_compile_features(${TAR} PRIVATE cxx_std_17)

    add_definitions(-DPROCESS_NAME="${TAR}")

    target_compile_options(${TAR} PRIVATE
        -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic
        -Wcast-align  -Woverloaded-virtual 
        -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op
        -Wconversion -Wsign-conversion -Wunused -Weffc++ -Werror
        -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2  -pipe -flto -Xlinker -Map=${TAR}.map
    )
#  -Wold-style-cast -Wconversion -Wsign-conversion -Wunused -Weffc++ -Werror
    target_link_libraries(${TAR} PRIVATE dl zmq pthread rt)

    set_target_properties(${TAR} PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
     )
    # Set safe install options (Yocto-compatible)
    install(TARGETS ${TAR} RUNTIME DESTINATION bin  )
    
   # Add post-install stripping using CMAKE_STRIP
   install(CODE "
   message(STATUS \"Stripping binary: bin/${TAR}\")
   execute_process(COMMAND ${CMAKE_STRIP} --strip-unneeded \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/${TAR})
   ")
   
endfunction()
