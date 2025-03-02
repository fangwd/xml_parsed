cmake_minimum_required(VERSION 3.10)
project(xmlcopy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

set(SOURCES
    main.cpp
    xml_doc_size.cpp
    xml_doc_copy.cpp
    xml_doc_wrap.cpp
)

add_executable(xmlcopy ${SOURCES})

target_include_directories(xmlcopy PRIVATE ${LIBXML2_INCLUDE_DIRS})
target_link_libraries(xmlcopy PRIVATE ${LIBXML2_LIBRARIES})

target_compile_options(xmlcopy PRIVATE ${LIBXML2_CFLAGS_OTHER})
