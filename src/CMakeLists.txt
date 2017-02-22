add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Boundary)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Control)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Entity)

set(SOURCE
${SOURCE}
${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/my_utils.cpp
PARENT_SCOPE
)

