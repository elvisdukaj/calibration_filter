cmake_minimum_required(VERSION 3.11)

project(calibration_filter LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB HEADERS_FILES "include/*.h*")
file(GLOB RESOURCES_FILES "resource/*.qrc")
file(GLOB SOURCE_FILES "src/*.cpp")
set(SOURCES ${HEADERS_FILES} ${SOURCE_FILES} ${RESOURCES_FILES})

add_executable(${PROJECT_NAME} "${SOURCES}")

target_include_directories(${PROJECT_NAME} PRIVATE
    include)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

find_package(Qt5 COMPONENTS Core Quick Multimedia REQUIRED)
find_package(OpenCV REQUIRED)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::Multimedia
    ${OpenCV_LIBS}
    )
