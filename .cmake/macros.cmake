set_property(GLOBAL PROPERTY LC_COLLECT_LIBS "")
set_property(GLOBAL PROPERTY LC_COLLECT_INCDIRS "")

macro(collect_lib)
  if(WITH_HEADERS)
    set_property(GLOBAL APPEND PROPERTY LC_COLLECT_LIBS ${ARGN})
  endif(WITH_HEADERS)
endmacro()

macro(find_header_files subdir header_files)
  file(
    GLOB_RECURSE file_paths CONFIGURE_DEPENDS
    LIST_DIRECTORIES false
    "${subdir}/*")

  foreach(file_path IN LISTS file_paths)
    get_filename_component(file_name "${file_path}" NAME)
    if(file_name MATCHES "[.]h$"
       OR file_name MATCHES "[.]hpp$"
       OR file_name MATCHES "[.]tcc$"
       OR file_name STREQUAL "LICENSE")
      file(RELATIVE_PATH rel_path "${subdir}" "${file_path}")
      list(APPEND ${header_files} "${rel_path}")
    endif()
  endforeach()
endmacro()

macro(install_headers subdir)
  if(WITH_HEADERS)
    set(SUBDIR ${subdir})
    set(HEADER_FILES)
    find_header_files(${SUBDIR} HEADER_FILES)

    cmake_path(GET SUBDIR FILENAME dir_name)
    set(install_root "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_ALIAS}/${dir_name}")
    set_property(
      GLOBAL APPEND PROPERTY LC_COLLECT_INCDIRS
                             "${CMAKE_INSTALL_PREFIX}/${install_root}")

    foreach(header_file IN LISTS HEADER_FILES)
      get_filename_component(header_dir "${header_file}" DIRECTORY)
      if(header_dir STREQUAL "")
        set(destination "${install_root}")
      else()
        set(destination "${install_root}/${header_dir}")
      endif()
      install(FILES "${header_file}" DESTINATION "${destination}")
    endforeach()
  endif(WITH_HEADERS)
endmacro()
