add_library(common STATIC
    common.h
    common.c)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})
