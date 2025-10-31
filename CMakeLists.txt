cmake_minimum_required(VERSION 3.16)

project(ECOSYSQT VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        Widget.cpp
        Widget.h
        DataStructures.h
        IBackendInterface.h
        TestBackend.cpp
        TestBackend.h
        my_resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ECOSYSQT
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(ECOSYSQT ${PROJECT_SOURCES})
endif()

target_link_libraries(ECOSYSQT PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(ECOSYSQT PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ECOSYSQT)
endif()