cmake_minimum_required(VERSION 3.5.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(vcpkg)

project(ottocento-engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(Warnings)
include(Sanitizers)
include(Coverage)
include(BuildType)
enable_warnings()
enable_sanitizers()
enable_coverage()
set_default_buildtype()

add_subdirectory(lib)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING)
   add_subdirectory(test)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
