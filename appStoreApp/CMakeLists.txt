cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME appStoreApp)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS WebSockets REQUIRED)
find_package(Qt5 COMPONENTS WebEngineWidgets REQUIRED)

#qt5_add_resources(RES ${CMAKE_CURRENT_SOURCE_DIR}/res/qml.qrc)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::WebSockets
    Qt5::WebEngineWidgets
    spdlog
)
