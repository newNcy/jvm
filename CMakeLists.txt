project(jvm)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions("-g")



include_directories(src)

aux_source_directory(src SRC)
aux_source_directory(src/native SRC_NATIVE)


add_library(native SHARED ${SRC_NATIVE})
add_executable(java ${SRC})
target_link_libraries(java zip dl ffi native pthread)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

aux_source_directory(class JAVA_SRC)
execute_process(COMMAND javac ${JAVA_SRC} )

