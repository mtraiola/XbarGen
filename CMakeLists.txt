# CMakeLists files in this project can
# refer to the root source directory of the project as ${XbarGen_SOURCE_DIR} and
# to the root binary directory of the project as ${XbarGen_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (XbarGen CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

SET(GCC_COVERAGE_COMPILE_FLAGS "-std=c++11 -D__cplusplus=201103L")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

add_executable(XbarGen ${HEADERS} ${SOURCE})

# Link the executable to the liblemon library. Since the liblemon library has public include directories we will use those link directories when building XbarGen

SET_TARGET_PROPERTIES(XbarGen PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (XbarGen ${CMAKE_SOURCE_DIR}/lemon_lib/libemon.a)

install(DIRECTORY DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Demo)
install(TARGETS XbarGen RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Demo)
FILE(GLOB files "${CMAKE_SOURCE_DIR}/demo_files/*.eqn" "${CMAKE_SOURCE_DIR}/VHDLrsrvdWords.dat")
INSTALL(FILES ${files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Demo)


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/XbarGen_doxy.doxyfile)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

endif()
