#Write compilation info to version.cpp file
string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")

IF(EXISTS ${DIR}/.git OR EXISTS ${DIR}/../.git)
    execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE BUILD_GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    execute_process(
            COMMAND git log -1 --format=%h
            OUTPUT_VARIABLE BUILD_GIT_COMMIT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set(BUILD_INFO "branch ${BUILD_GIT_BRANCH} - commit ${BUILD_GIT_COMMIT}")
else()
    set(BUILD_INFO "Release")
endif()

configure_file(${DIR}/build_constants.cpp.in ${DIR}/build_constants.cpp)

message(STATUS "Build date: ${BUILD_DATE}")
message(STATUS "Build info: ${BUILD_INFO}")
