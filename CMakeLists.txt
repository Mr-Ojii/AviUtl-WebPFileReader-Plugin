cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(webpinput CXX)

find_package(WebP REQUIRED CONFIG)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/main.def
    src/FileHandle.cpp
)

execute_process(
    COMMAND git rev-list --count HEAD
    OUTPUT_VARIABLE git_rev
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config.tmp" "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/aviutl_exedit_sdk)

target_include_directories(${PROJECT_NAME} PRIVATE ${WEBP_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE "$ENV{CMAKE_PREFIX_PATH}/lib")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    target_link_libraries(${PROJECT_NAME} PRIVATE libwebpdecoder libwebpdemux libwebp)
    target_compile_options(${PROJECT_NAME} PRIVATE /source-charset:utf-8 /execution-charset:shift_jis)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    target_link_libraries(${PROJECT_NAME} PRIVATE webpdecoder webpdemux webp)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--enable-stdcall-fixup -static-libstdc++ -static)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".aui")
