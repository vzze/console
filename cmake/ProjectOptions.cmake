include(CheckIPOSupported)

include(cmake/CompilerWarnings.cmake)
include(cmake/DependencySolver.cmake)
include(cmake/StaticAnalyzers.cmake)

macro(set_parallel_level)
    cmake_host_system_information(RESULT CORES QUERY NUMBER_OF_PHYSICAL_CORES)

    if(NOT ${CORES} EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${CORES})
        message(STATUS "${PROJECT_NAME}: Build in parallel enabled with ${CORES} cores")
    else()
        message(STATUS "${PROJECT_NAME}: Build in parallel disabled")
    endif()
endmacro()

macro(enable_ipo)
    check_ipo_supported(RESULT SUPPORTED OUTPUT ERROR)

    if(SUPPORTED)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            find_program(GCC_AR gcc-ar)

            if(GCC_AR)
                message(STATUS "${PROJECT_NAME}: GCC detected: setting up CMake AR")

                set(CMAKE_AR ${GCC_AR})
                set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
                set(CMAKE_C_ARCHIVE_FINISH true)

                set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
                message(STATUS "${PROJECT_NAME}: IPO / LTO Enabled")
            else()
                message(SEND_ERROR "${PROJECT_NAME}: gcc-ar is needed for LTO optimization")
            endif()
        else()
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
            message(STATUS "${PROJECT_NAME}: IPO / LTO Enabled")
        endif()
    else()
        message(STATUS "${PROJECT_NAME}: IPO / LTO not supported: ${ERROR}")
    endif()
endmacro()

macro(set_cmp0141)
    cmake_policy(SET CMP0141 NEW)
    set(
        CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
    )
endmacro()

macro(set_project_options INCLUDE_DIRS DEPENDENCY_DIR)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set_parallel_level()
    enable_ipo()

    if(MSVC AND POLICY CMP0141)
        set_cmp0141()
    endif()

    add_library(${PROJECT_NAME}_PROJECT_WARNINGS INTERFACE)
    set_compiler_warnings(${PROJECT_NAME}_PROJECT_WARNINGS)

    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/${DEPENDENCY_DIR}" AND NOT ${DEPENDENCY_DIR} STREQUAL "")
        message(STATUS "${PROJECT_NAME}: Searching for dependencies in: ${CMAKE_SOURCE_DIR}/${DEPENDENCY_DIR}")
        solve_dependencies("${CMAKE_SOURCE_DIR}/${DEPENDENCY_DIR}")
    endif()

    # Everything after dependencies is only enabled
    # for the project code, the user shouldn't worry
    # about warnings from other libs

    enable_clang_tidy()

    add_subdirectory(cmake/config)

    add_library(${PROJECT_NAME}_PROJECT_OPTIONS INTERFACE)

    set(PROJECT_INCLUDE_DIRS ${INCLUDE_DIRS})
    list(APPEND PROJECT_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/internal")
    message(STATUS "${PROJECT_NAME}: Include directories set to: ${PROJECT_INCLUDE_DIRS}")

    target_include_directories(${PROJECT_NAME}_PROJECT_OPTIONS INTERFACE ${PROJECT_INCLUDE_DIRS})

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endmacro()
