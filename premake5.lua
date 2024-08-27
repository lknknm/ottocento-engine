-- premake5.lua
----------------------------
-- Environment variables
VULKAN_SDK = os.getenv("VULKAN_SDK")

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

   files { "./src/*.h", "./src/*.cpp", "./src/*.hpp", "./src/*.cxx", "./external/imgui/*.cpp" }
   
   -- Requires to install the VulkanSDK package.
   if os.host() == "windows" then
        system "windows"
        externalincludedirs  { "external/glfw-3.4.bin.WIN64/include", "%{VULKAN_SDK}/Include", "external/glm", "external/stb", "external/imgui" }
        libdirs { "external/glfw-3.4.bin.WIN64/lib-vc2022", "%{VULKAN_SDK}/Lib" }
        links { "glfw3", "vulkan-1" }
        printf("windows setup")
   end
  
   -- Requires to install the GLFW library as 'sudo apt install libglfw3-dev'. 
   -- Refer to your distro GLFW and VulkanSDK packages for more info.
    if os.host() == "linux" then
        system "linux"
        GLFW = os.getenv("GLFW")
        externalincludedirs  { "%{GLFW}/include", "%{VULKAN_SDK}/Include", "external/**" }
        libdirs { "%{GLFW}/lib", "%{VULKAN_SDK}/Lib" }
        links { "GL", "glfw", "vulkan" , "dl", "X11", "pthread", "Xxf86vm",  "Xrandr", "Xi" }
        os.rmdir("external/glfw-3.4.bin.WIN64")
        printf("linux setup")
    end

   if os.host() == "macosx" then
      -- Assume glfw and vulkan are already in available in the environment
      externalincludedirs  { "external/glm", "external/stb", "external/imgui" }
      -- libdirs { "/usr/local/lib" }
      links { "glfw", "vulkan" , "dl",  "pthread" }
      printf("MacOS setup")
   end


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      symbols "Off"