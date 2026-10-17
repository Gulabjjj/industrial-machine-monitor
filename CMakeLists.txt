cmake_minimum_required(VERSION 3.16)
project(IndustrialMachineMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(IndustrialMachineMonitor
    src/main.cpp
    src/EventLog.h        src/EventLog.cpp
    src/Machine.h         src/Machine.cpp
    src/MachineWidget.h   src/MachineWidget.cpp
    src/MainWindow.h      src/MainWindow.cpp
)

target_link_libraries(IndustrialMachineMonitor PRIVATE Qt6::Widgets)

if(MSVC)
    target_compile_options(IndustrialMachineMonitor PRIVATE /W4)
else()
    target_compile_options(IndustrialMachineMonitor PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(IndustrialMachineMonitor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

install(TARGETS IndustrialMachineMonitor RUNTIME DESTINATION bin)
