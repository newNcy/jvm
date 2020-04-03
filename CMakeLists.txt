project(jvm)

set(CMAKE_CXX_STANDARD 11)
add_definitions("-g")
include_directories(src)
aux_source_directory(src SRC)
add_executable(jvm ${SRC})
target_link_libraries(jvm zip dl ffi)

