-- premake5.lua
----------------------------
VULKAN_SDK = os.getenv("VULKAN_SDK")
GLFW = os.getenv("GLFW")

----------------------------
workspace "vulkan-tutorial"
   configurations { "Debug", "Release" }
   architecture "x64"
   
----------------------------
project "vulkan-tutorial"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "c++17" -- apply to all toolsets and generators

   files { "./src/*.h", "./src/*.cpp", "./src/*.hpp", "./src/*.cxx" }
   
   includedirs { "external/glfw-3.4.bin.WIN64/include", "%{VULKAN_SDK}/Include", "external/glm" }
   libdirs { "external/glfw-3.4.bin.WIN64/lib-vc2022", "%{VULKAN_SDK}/Lib" }
   links { "glfw3", "vulkan-1" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      symbols "Off"