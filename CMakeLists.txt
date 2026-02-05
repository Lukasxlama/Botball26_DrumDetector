cmake_minimum_required(VERSION 3.14)
project(DrumDetector)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
)

FetchContent_MakeAvailable(json spdlog)

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE LIB_SOURCES
        "impl/*.cpp"
        "include/*.hpp"
)

add_library(DrumDetector STATIC ${LIB_SOURCES})

target_include_directories(DrumDetector PUBLIC include)

target_link_libraries(DrumDetector
        PUBLIC
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        PRIVATE
        ${OpenCV_LIBS}
)