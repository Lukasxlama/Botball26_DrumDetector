cmake_minimum_required(VERSION 3.11)
project(DrumDetector)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/include.zip
)
FetchContent_MakeAvailable(json)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
endif()

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE LIB_SOURCES
        "impl/*.cpp"
        "include/*.hpp"
)

add_library(DrumDetector STATIC ${LIB_SOURCES})

target_include_directories(DrumDetector PUBLIC
        include
        ${json_SOURCE_DIR}/include
)

target_link_libraries(DrumDetector
        PRIVATE
        ${OpenCV_LIBS}
)