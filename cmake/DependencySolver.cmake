macro(solve_dependencies DIRECTORY)
    macro(dir_list RESULT CURDIR)
        file(GLOB CHILDREN RELATIVE ${CURDIR} "${CURDIR}/*")

        set(DIRLIST "")

        foreach(CHILD ${CHILDREN})
            if(IS_DIRECTORY ${CURDIR}/${CHILD})
                list(APPEND DIRLIST ${CHILD})
            endif()
        endforeach()

        set(${RESULT} ${DIRLIST})
    endmacro()

    dir_list(SUBDIRS ${DIRECTORY})

    foreach(DIR ${SUBDIRS})
        message(STATUS "${PROJECT_NAME}: Dependency: ${DIR}")
        add_subdirectory("${DIRECTORY}/${DIR}")
    endforeach()
endmacro()
