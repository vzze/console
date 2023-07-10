add_executable(callbacks "callbacks.cc")

target_link_libraries(callbacks PUBLIC console PRIVATE console_PROJECT_WARNINGS)

add_executable(blank "blank.cc")

target_link_libraries(blank PUBLIC console PRIVATE console_PROJECT_WARNINGS)
