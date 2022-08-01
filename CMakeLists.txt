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
