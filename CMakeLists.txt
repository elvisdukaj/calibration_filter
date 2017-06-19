cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_STANDARD 14)
project(calibration_filter LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Quick Multimedia REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include)

file(GLOB HEADERS_FILES "include/*.h*")
file(GLOB RESOURCES_FILES "resource/*.qrc")
file(GLOB SOURCE_FILES "src/*.cpp")
set(SOURCES ${HEADERS_FILES} ${SOURCE_FILES} ${RESOURCES_FILES})

add_executable(${PROJECT_NAME} "${SOURCES}")

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Quick
    Qt5::Multimedia
    ${OpenCV_LIBS}
    )
