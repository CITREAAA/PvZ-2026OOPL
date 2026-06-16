cmake_minimum_required(VERSION 3.16)

# 強制讓所有下載下來的第三方庫都採用 3.5 以上的現代 Policy 執行，免除 SDL2 報錯
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(PvZ)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0135 NEW)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
        ptsd
        URL         https://github.com/ntut-open-source-club/practical-tools-for-simple-design/archive/refs/tags/v0.2.zip
        SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/PTSD
)

FetchContent_MakeAvailable(ptsd)

include(files.cmake)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
list(TRANSFORM SRC_FILES PREPEND ${SRC_DIR}/)
list(TRANSFORM INCLUDE_FILES PREPEND ${INCLUDE_DIR}/)

add_executable(${PROJECT_NAME}
        ${SRC_DIR}/main.cpp
        ${SRC_FILES}
        ${HEADER_FILES}
        include/App.hpp
        src/App.cpp
        include/gameMap.hpp
        include/gameEntity.hpp
        src/gameMap.cpp
        include/gameManager.hpp
        include/zombie.hpp
        src/plant.cpp
        include/sun.hpp
        src/sun.cpp
        src/pea.cpp
        include/pea.hpp
        include/seedBank.hpp
        src/zombie.cpp
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")
else()
    message(FATAL_ERROR "relative RESOURCE_DIR is WIP, Please use `-DCMAKE_BUILD_TYPE=Debug` build for now.")
endif()

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${DEPENDENCY_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/PTSD/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
        SDL2::SDL2main
        PTSD
)

# 🚩 【救命的終極大招】：直接在編譯資料夾底下建立資源捷徑！
# 不管 CLion 在哪裡執行，執行檔旁邊都一定會有這個捷徑，直接通往你的 resources
file(CREATE_LINK
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "${CMAKE_CURRENT_BINARY_DIR}/resources"
        SYMBOLIC
)