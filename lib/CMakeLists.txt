include(FindVulkan)
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

file(GLOB sources CONFIGURE_DEPENDS *.cxx)

add_library(lib_ottocento_engine ${sources})

set_target_properties(lib_ottocento_engine PROPERTIES CXX_STANDARD 20)

target_include_directories(lib_ottocento_engine PUBLIC include)
target_include_directories(lib_ottocento_engine SYSTEM PRIVATE
   ${Stb_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/external/randutils)

target_link_libraries(lib_ottocento_engine PUBLIC
   glm::glm
   Vulkan::Vulkan
   glfw
   volk::volk
   imgui::imgui
   fmt::fmt
   volk::volk_headers
   tinyobjloader::tinyobjloader
)
