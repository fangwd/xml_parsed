cmake_minimum_required(VERSION 3.10)
project(parsed)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

set(COMMON_SOURCES
    xml_doc_size.cpp
    xml_doc_copy.cpp
    xml_doc_wrap.cpp
)

add_executable(parsed main.cpp ${COMMON_SOURCES})
add_executable(test test.cpp ${COMMON_SOURCES})

target_include_directories(parsed PRIVATE ${LIBXML2_INCLUDE_DIRS})
target_include_directories(test PRIVATE ${LIBXML2_INCLUDE_DIRS})

target_link_libraries(parsed PRIVATE ${LIBXML2_LIBRARIES})
target_link_libraries(test PRIVATE ${LIBXML2_LIBRARIES})

target_compile_options(parsed PRIVATE ${LIBXML2_CFLAGS_OTHER})
target_compile_options(test PRIVATE ${LIBXML2_CFLAGS_OTHER})
