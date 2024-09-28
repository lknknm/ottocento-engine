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

target_include_directories(lib_ottocento_engine PUBLIC
   ${Stb_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/external/randutils
   include
)
target_link_libraries(lib_ottocento_engine PUBLIC
   Vulkan::Vulkan
   glm::glm
   glfw
   volk::volk
   volk::volk_headers
   imgui::imgui
   tinyobjloader::tinyobjloader
   fmt::fmt
)
