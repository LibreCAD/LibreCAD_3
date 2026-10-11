cmake_minimum_required(VERSION 3.15...3.27)

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

    # Resolve this package at runtime from the executable/bundle. Unix
    # installs place it under share/librecad; Windows installs beside prefix.
    if(MSVC)
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUIPy"
                DESTINATION "."
                PATTERN "CMakeLists.txt" EXCLUDE
                PATTERN "__pycache__" EXCLUDE)
    else()
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUIPy"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/librecad"
                PATTERN "CMakeLists.txt" EXCLUDE
                PATTERN "__pycache__" EXCLUDE)
    endif()
endif()
