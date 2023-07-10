set(CONSOLE "common.cc;helpers.cc")

if(WIN32)
    list(APPEND CONSOLE "win32.cc")
elseif(UNIX)
    list(APPEND CONSOLE "unix.cc")
endif()

add_library(console ${CONSOLE})

target_link_libraries(console PUBLIC console_PROJECT_OPTIONS PRIVATE console_PROJECT_WARNINGS)
