option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)
if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    add_compile_options("--coverage")
    add_link_options("--coverage")
  endif()

  find_program(GCOVR_EXECUTABLE NAMES gcovr)
  if(NOT GCOVR_EXECUTABLE)
    message(WARNING "Need gcovr to create a full report in html form")
  else()
    # We want llvm-cov, but on mac it's aliased to just 'gcov'
    find_program(GCOV_EXECUTABLE NAMES llvm-cov gcov)
    if(NOT GCOV_EXECUTABLE)
      message(FATAL_ERROR "No coverage tool available")
    elseif(GCOV_EXECUTABLE MATCHES "llvm-cov")
      set(GCOV_EXECUTABLE "${GCOV_EXECUTABLE} gcov")
    endif()

    # Define where the report should go
    set(COVERAGE_REPORT_DIR "${CMAKE_BINARY_DIR}/coverage_report")
    set(HTML_REPORT_FILE "${COVERAGE_REPORT_DIR}/index.html")

    set(HTML_THEME "" CACHE INTERNAL "The string that will contain the full theme")

    # Set coverage report style components
    set(HTML_HL_COLOR green CACHE STRING "Color style")
    set(HTML_GITHUB_STYLE OFF CACHE BOOL "Use github styling")
    set_property(CACHE HTML_HL_COLOR PROPERTY STRINGS "green" "blue")

    if(HTML_GITHUB_STYLE)
      string(APPEND HTML_THEME "github.")
      set(HTML_DARK_MODE OFF CACHE BOOL "Light/Dark mode")
      if(HTML_DARK_MODE)
        string(APPEND HTML_THEME "dark-")
      endif()
    endif()
    string(APPEND HTML_THEME ${HTML_HL_COLOR})

    set(HTML_USE_NESTED OFF CACHE BOOL "Use folder structure")
    if(HTML_USE_NESTED)
      set(HTML_DETAILS_OR_NESTED "nested")
    else()
      set(HTML_DETAILS_OR_NESTED "details")
    endif()

    # Generate coverage report with results from tests
    add_custom_target(
      coverage_report
      COMMAND ${CMAKE_COMMAND} -E make_directory "${COVERAGE_REPORT_DIR}"
      COMMAND
        ${GCOVR_EXECUTABLE} --gcov-executable "${GCOV_EXECUTABLE}" #
        --root "${CMAKE_SOURCE_DIR}" # root directory
        --html-"${HTML_DETAILS_OR_NESTED}" "${HTML_REPORT_FILE}" #
        --html-theme ${HTML_THEME} #
        # Filter options can be added here, e.g., --filter "src/.*"
        -e \".*vcpkg_installed/.*\" # Exclude files
      # Add --exclude-unreachable-branches if desired
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      COMMENT "Generating detailed HTML coverage report using gcovr..."
    )
  endif()

  # Clean up all coverage artifacts
  find_program(FD_EXECUTABLE fd)
  if(FD_EXECUTABLE)
    add_custom_target(
      clean_profile_data
      COMMAND fd -tf \".*\\.gc[da|no]\" | xargs rm
    )
  endif()

  # Cleanup rules
  file(GLOB_RECURSE COVERAGE_ARTIFACTS "${CMAKE_CURRENT_BINARY_DIR}/*.gcno" "${CMAKE_CURRENT_BINARY_DIR}/*.gcda")
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${COVERAGE_ARTIFACTS}")
endif()
