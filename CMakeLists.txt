cmake_minimum_required(VERSION 3.15)

project(inifile)

set(HEADERS
    include/inifile/ini.h
)

set(SOURCES
    src/ini.cpp
)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
option(USE_NARROW_ONLY "Do not use wchar_t on Windows" OFF)
target_compile_definitions(${PROJECT_NAME} PUBLIC $<IF:$<BOOL:${USE_NARROW_ONLY}>,USE_NARROW_ONLY,>)

add_executable(IniTest src/main.cpp)
target_link_libraries(IniTest PUBLIC ${PROJECT_NAME})
