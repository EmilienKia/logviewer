cmake_minimum_required(VERSION 3.0)

project(logviewer)

set(SRCS
        src/app.cpp
        src/data.cpp
        src/fdartprov.cpp
        src/files.cpp
        src/frame.cpp
        src/model.cpp
        src/parser.cpp
        )

set(HEADERS
        src/app.hpp
        src/data.hpp
        src/fdartprov.hpp
        src/files.hpp
        src/frame.hpp
        src/model.hpp
        src/parser.hpp
        )

set(wxWidgets_CONFIGURATION gtku)
find_package(wxWidgets COMPONENTS adv aui ribbon core base REQUIRED)
include(${wxWidgets_USE_FILE})

add_compile_definitions(ICONSETDIR="/usr/local/share/logviewer/icons")

add_executable(logviewer ${SRCS} ${HEADERS})
target_link_libraries(logviewer ${wxWidgets_LIBRARIES})

