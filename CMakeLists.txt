cmake_minimum_required(VERSION 3.15)
project(inifile)

include(GenerateExportHeader)

set(HEADERS
    include/inifile/file_handler.h
    include/inifile/ini.h
    include/inifile/strings_handler.h
)

set(SOURCES
    src/ini.cpp
)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
generate_export_header(
    ${PROJECT_NAME}
    BASE_NAME INIFILE
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/generated/inifile/inifile_export.h"
    EXPORT_MACRO_NAME INIFILE_API
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    $<INSTALL_INTERFACE:include>
    include
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

option(INI_BUILD_TEST "Build inifile test" OFF)
option(USE_NARROW_ONLY "Do not use wchar_t on Windows" OFF)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<IF:$<BOOL:${USE_NARROW_ONLY}>,USE_NARROW_ONLY,>)
if(INI_BUILD_TEST)
    enable_testing()
    add_executable(IniTest src/main.cpp)
    target_link_libraries(IniTest PUBLIC ${PROJECT_NAME})
    add_test(NAME IniTest COMMAND $<TARGET_FILE:IniTest>)
endif()
