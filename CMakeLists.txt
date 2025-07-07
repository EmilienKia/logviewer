cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

project(logviewer)

include(cmake/embed_resources.cmake)

set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/share/icons")
set(EMBEDDED_RESOURCE_FILE "${CMAKE_BINARY_DIR}/embedded_resources.h")
embed_resources(logviewer ${RESOURCES_DIR} ${EMBEDDED_RESOURCE_FILE})

set(SRCS
        src/app.cpp
        src/data.cpp
        src/fdartprov.cpp
        src/files.cpp
        src/frame.cpp
        src/helpers.cpp
        src/model.cpp
        src/parser.cpp
        src/embedartprov.cpp
        )

set(HEADERS
        src/app.hpp
        src/data.hpp
        src/fdartprov.hpp
        src/files.hpp
        src/frame.hpp
        src/helpers.hpp
        src/model.hpp
        src/parser.hpp
        src/embedartprov.hpp
        )

set(wxWidgets_CONFIGURATION gtku)
find_package(wxWidgets COMPONENTS adv aui ribbon core base REQUIRED)
include(${wxWidgets_USE_FILE})

add_compile_definitions(ICONSETDIR="/usr/local/share/logviewer/icons")

find_package(PkgConfig REQUIRED)
pkg_check_modules(PCRE2 libpcre2-8 REQUIRED IMPORTED_TARGET)
if(${PCRE2_FOUND})
        MESSAGE(INFO "REGEX: PCRE2 POSIX found")
else()
        MESSAGE(FATAL_ERROR "REGEX: PCRE2 POSIX not found")
endif()
#set(PCRE2_USE_STATIC_LIBS ON)
#find_package(PCRE2 CONFIG REQUIRED)

add_executable(logviewer ${SRCS} ${HEADERS})
target_include_directories(logviewer PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(logviewer ${wxWidgets_LIBRARIES} ${PCRE2_LIBRARIES})




add_executable(utest
        tests/catch.hpp
        tests/runner.cpp

        src/parser.hpp
        src/parser.cpp
        src/data.hpp
        src/data.cpp

        tests/test_parser.cpp
        tests/test_parser_date.cpp
        tests/test_regex.cpp
)
target_link_libraries(utest ${PCRE2_LIBRARIES})
add_test(NAME utest COMMAND utest)
