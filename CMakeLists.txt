cmake_minimum_required(VERSION 3.5.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(vcpkg)

project(ottocento-engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
include(FindVulkan)

include(Warnings)
include(Sanitizers)
include(Coverage)
include(BuildType)
enable_warnings()
enable_sanitizers()
enable_coverage()
set_default_buildtype()

find_package(Vulkan REQUIRED)
find_package(glfw3 3.4 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# add_subdirectory(external/volk)
file(GLOB sources CONFIGURE_DEPENDS src/*.cxx)
add_executable(ottocento-engine ${sources})

target_link_libraries(ottocento-engine PRIVATE Vulkan::Vulkan glm::glm glfw volk::volk volk::volk_headers imgui::imgui)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
