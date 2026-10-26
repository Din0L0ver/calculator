cmake_minimum_required(VERSION 3.16)
project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

option(CALC_SANITIZERS "Enable address and undefined behavior checks" OFF)
if(CALC_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(calc_core src/Operation.cpp src/Command.cpp src/Parser.cpp src/PluginLoader.cpp)
target_include_directories(calc_core PUBLIC include)
target_link_libraries(calc_core PRIVATE ${CMAKE_DL_LIBS})

add_executable(calc src/main.cpp)
target_link_libraries(calc PRIVATE calc_core)
# $<0:> предотвращает добавление Debug/Release в многоконфигурационных генераторах.
set_target_properties(calc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin$<0:>")

function(add_calc_plugin target source directory)
    add_library(${target} SHARED ${source})
    target_include_directories(${target} PRIVATE include plugins)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        RUNTIME_OUTPUT_DIRECTORY "${directory}$<0:>"
        LIBRARY_OUTPUT_DIRECTORY "${directory}$<0:>")
endfunction()

add_calc_plugin(funcsin plugins/Sin.cpp "${CMAKE_BINARY_DIR}/bin/plugins")
add_calc_plugin(funcdeg plugins/Power.cpp "${CMAKE_BINARY_DIR}/bin/plugins")
add_calc_plugin(funcln plugins/Ln.cpp "${CMAKE_BINARY_DIR}/bin/plugins")

include(CTest)
if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_calc_plugin(testfixture tests/Fixture.cpp "${CMAKE_BINARY_DIR}/fixtures")
    add_calc_plugin(missingexport tests/MissingExport.cpp "${CMAKE_BINARY_DIR}/fixtures")
    add_executable(lifetime_test tests/Lifetime.cpp)
    target_link_libraries(lifetime_test PRIVATE calc_core)
    add_test(NAME command_lifetime COMMAND lifetime_test "${CMAKE_BINARY_DIR}/bin/plugins")
    add_test(NAME calculator_integration COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration.py" "$<TARGET_FILE:calc>"
        "$<TARGET_FILE:funcsin>" "$<TARGET_FILE:funcdeg>" "$<TARGET_FILE:funcln>"
        "$<TARGET_FILE:testfixture>" "$<TARGET_FILE:missingexport>")
endif()
