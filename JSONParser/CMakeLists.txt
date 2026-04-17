cmake_minimum_required(VERSION 3.13)
project(json_parser)

set(CPP_MODE ON)
if(CPP_MODE)
  set(FB_EXT ".cpp")
else()
  set(FB_EXT ".c")
endif()

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(L_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/json.l)
set(Y_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/json.y)
set(L_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/json.lex${FB_EXT})
set(Y_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/json.tab${FB_EXT})

flex_target(Lexer ${L_SOURCES} ${L_OUTPUTS})
bison_target(Parser ${Y_SOURCES} ${Y_OUTPUTS})
add_flex_bison_dependency(Lexer Parser)

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES 
    src/main.cpp 
    src/json_ast.cpp
    ${FLEX_Lexer_OUTPUTS} 
    ${BISON_Parser_OUTPUT_SOURCE})

add_executable(json_parser ${SOURCES})
set_target_properties(json_parser PROPERTIES CXX_STANDARD 11)