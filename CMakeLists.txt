project(jvm)

set(CMAKE_CXX_STANDARD 11)
add_definitions("-g")



include_directories(src)

aux_source_directory(src SRC)
aux_source_directory(src/native SRC_NATIVE)


add_library(native SHARED ${SRC_NATIVE})
add_executable(java ${SRC})
target_link_libraries(java zip dl ffi native)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

execute_process(COMMAND javac ${JAVA_SRC} )

