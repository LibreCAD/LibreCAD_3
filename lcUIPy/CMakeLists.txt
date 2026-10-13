cmake_minimum_required(VERSION 3.28)

message("***** LibreCAD Python scripts *****")

if(WITH_PYTHONSCRIPT)
    # Mirror the package into the build output so executables resolve it
    # relative to their own directory. This keeps developer builds working
    # without baking the source-tree path into installed binaries.
    add_custom_target(lcuipy_build_tree ALL
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_SOURCE_DIR}/lcUIPy"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lcUIPy"
        COMMENT "Copying lcUIPy into the build runtime directory"
        VERBATIM)

    # Keep the package beside staged Python and its license on every platform.
    # Both runtime locators resolve this prefix-relative share/librecad layout;
    # a Windows-only prefix/lcUIPy split breaks the installed smoke's contract.
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUIPy"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/librecad"
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "__pycache__" EXCLUDE)
endif()
