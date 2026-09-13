cmake_minimum_required(VERSION 3.24)
project(ElysiumProduction VERSION 0.24.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ELYSIUM_BUILD_CLIENT "Build the real Raylib + EnTT Windows-capable client" ON)
option(ELYSIUM_BUILD_TESTS "Compile and register all recovered test translation units" ON)
option(ELYSIUM_BUILD_TOOLS "Compile recovered developer tools" ON)
option(ELYSIUM_FETCH_DEPENDENCIES "Fetch pinned Raylib and EnTT with CMake FetchContent" ON)
option(ELYSIUM_ENFORCE_SOURCE_COVERAGE "Fail configure if a recovered .cpp is missing from the production inventory" ON)

if(NOT ELYSIUM_BUILD_CLIENT)
    message(FATAL_ERROR "The production-all graph requires ELYSIUM_BUILD_CLIENT=ON so all runtime translation units compile against real Raylib + EnTT.")
endif()

find_package(Threads REQUIRED)
include(cmake/ProductionSources.cmake)

# Production invariant: every recovered translation unit is explicitly classified.
if(ELYSIUM_ENFORCE_SOURCE_COVERAGE)
    file(GLOB_RECURSE _actual_runtime CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
    list(REMOVE_ITEM _actual_runtime "src/main.cpp")
    list(SORT _actual_runtime)
    set(_declared_runtime ${ELYSIUM_PRODUCTION_RUNTIME_SOURCES})
    list(SORT _declared_runtime)
    if(NOT "${_actual_runtime}" STREQUAL "${_declared_runtime}")
        message(FATAL_ERROR "Production runtime source inventory is stale. Run: python tools/generate_production_sources.py")
    endif()

    file(GLOB_RECURSE _actual_tests CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "tests/*.cpp")
    list(SORT _actual_tests)
    set(_declared_tests ${ELYSIUM_PRODUCTION_TEST_MAIN_SOURCES} ${ELYSIUM_PRODUCTION_TEST_SUPPORT_SOURCES})
    list(SORT _declared_tests)
    if(NOT "${_actual_tests}" STREQUAL "${_declared_tests}")
        message(FATAL_ERROR "Production test source inventory is stale. Run: python tools/generate_production_sources.py")
    endif()

    file(GLOB_RECURSE _actual_tools CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "tools/*.cpp")
    list(SORT _actual_tools)
    set(_declared_tools ${ELYSIUM_PRODUCTION_TOOL_SOURCES})
    list(SORT _declared_tools)
    if(NOT "${_actual_tools}" STREQUAL "${_declared_tools}")
        message(FATAL_ERROR "Production tool source inventory is stale. Run: python tools/generate_production_sources.py")
    endif()
endif()

if(ELYSIUM_FETCH_DEPENDENCIES)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
    )
    FetchContent_Declare(
        EnTT
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.13.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib EnTT)
else()
    find_package(raylib 5.5 CONFIG REQUIRED)
    find_package(EnTT 3.13.2 CONFIG REQUIRED)
endif()

# All 1,587 non-main runtime .cpp files compile exactly once here. The final game
# executable receives the object files directly so the linker sees every module;
# this deliberately prevents a static archive from hiding duplicate symbols or
# unreferenced donor implementations.
add_library(elysium_production_objects OBJECT ${ELYSIUM_PRODUCTION_RUNTIME_SOURCES})
target_include_directories(elysium_production_objects PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(elysium_production_objects PUBLIC Threads::Threads EnTT::EnTT raylib)
target_compile_definitions(elysium_production_objects PRIVATE ELYSIUM_PRODUCTION_BUILD=1)

if(WIN32)
    target_compile_definitions(elysium_production_objects PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(MSVC)
    target_compile_options(elysium_production_objects PRIVATE /W3 /permissive- /bigobj /utf-8 /MP)
else()
    target_compile_options(elysium_production_objects PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Convenience archive for tests/tools. The game itself links the object list
# directly to force full production link coverage.
add_library(elysium_runtime STATIC $<TARGET_OBJECTS:elysium_production_objects>)
target_include_directories(elysium_runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(elysium_runtime PUBLIC Threads::Threads EnTT::EnTT raylib)

add_executable(elysium src/main.cpp $<TARGET_OBJECTS:elysium_production_objects>)
target_include_directories(elysium PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(elysium PRIVATE Threads::Threads EnTT::EnTT raylib)
set_target_properties(elysium PROPERTIES OUTPUT_NAME "Elysium")

if(MSVC)
    target_compile_options(elysium PRIVATE /W3 /permissive- /bigobj /utf-8)
else()
    target_compile_options(elysium PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Every test .cpp is compiled. Files without main() are grouped into test-support;
# files with main() become individual CTest executables.
if(ELYSIUM_BUILD_TESTS)
    enable_testing()
    if(ELYSIUM_PRODUCTION_TEST_SUPPORT_SOURCES)
        add_library(elysium_test_support STATIC ${ELYSIUM_PRODUCTION_TEST_SUPPORT_SOURCES})
        target_include_directories(elysium_test_support PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_link_libraries(elysium_test_support PRIVATE elysium_runtime Threads::Threads EnTT::EnTT raylib)
        if(MSVC)
            target_compile_options(elysium_test_support PRIVATE /W3 /permissive- /bigobj /utf-8 /MP)
        endif()
    endif()

    foreach(test_source IN LISTS ELYSIUM_PRODUCTION_TEST_MAIN_SOURCES)
        get_filename_component(test_name "${test_source}" NAME_WE)
        set(test_target "elysium_test_${test_name}")
        add_executable(${test_target} "${test_source}")
        target_include_directories(${test_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        if(TARGET elysium_test_support)
            target_link_libraries(${test_target} PRIVATE elysium_test_support)
        endif()
        target_link_libraries(${test_target} PRIVATE elysium_runtime Threads::Threads EnTT::EnTT raylib)
        if(MSVC)
            target_compile_options(${test_target} PRIVATE /W3 /permissive- /bigobj /utf-8)
        endif()
        add_test(NAME ${test_name} COMMAND ${test_target})
    endforeach()
endif()

if(ELYSIUM_BUILD_TOOLS)
    foreach(tool_source IN LISTS ELYSIUM_PRODUCTION_TOOL_SOURCES)
        get_filename_component(tool_name "${tool_source}" NAME_WE)
        set(tool_target "elysium_tool_${tool_name}")
        add_executable(${tool_target} "${tool_source}")
        target_include_directories(${tool_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_link_libraries(${tool_target} PRIVATE elysium_runtime Threads::Threads EnTT::EnTT raylib)
        if(MSVC)
            target_compile_options(${tool_target} PRIVATE /W3 /permissive- /bigobj /utf-8)
        endif()
    endforeach()
endif()

# Portable release layout. Asset/config/license directories are optional at
# configure time but are copied into the installed Windows bundle when present.
install(TARGETS elysium RUNTIME DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION assets OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/config/" DESTINATION config OPTIONAL)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/licenses/" DESTINATION licenses OPTIONAL)
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/PRODUCTION_BUILD.md"
    DESTINATION .
    OPTIONAL
)

set(CPACK_PACKAGE_NAME "Elysium")
set(CPACK_PACKAGE_VENDOR "Elysium Project")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "ZIP")
include(CPack)
