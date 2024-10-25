-- premake5.lua
----------------------------
-- Environment variables
VULKAN_SDK = os.getenv("VULKAN_SDK")

----------------------------
workspace "ottocento-engine"
   configurations { "Debug", "Release" }
   architecture "x64"
   
----------------------------
project "ottocento-engine"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "c++20" -- apply to all toolsets and generators

   files { "./src/*.h", "./src/*.cpp", "./src/*.hpp", "./src/*.cxx", "./external/imgui/*.cpp" }
   
   -- Requires to install the VulkanSDK package.
   if os.host() == "windows" then
        system "windows"
        externalincludedirs  { "external/glfw-3.4.bin.WIN64/include", "%{VULKAN_SDK}/Include", "external/volk", "external/glm", "external/stb", "external/imgui", "external/ifcopenshell/include", "external/boost_1_86" }
        libdirs { "external/glfw-3.4.bin.WIN64/lib-vc2022", "%{VULKAN_SDK}/Lib", "external/ifcopenshell/lib" }
        links { "glfw3" }
        printf("windows setup")
   end
  
   -- Requires to install the GLFW library as 'sudo apt install libglfw3-dev'. 
   -- Refer to your distro GLFW and VulkanSDK packages for more info.
    if os.host() == "linux" then
        system "linux"
		  enablewarnings { "all", "extra", "shadow", "cast-align", "null-dereference" }
        GLFW = os.getenv("GLFW")
        externalincludedirs  { "%{GLFW}/include", "%{VULKAN_SDK}/Include", "external/**" }
        libdirs { "%{GLFW}/lib", "%{VULKAN_SDK}/Lib" }
        links { "GL", "glfw", "vulkan" , "dl", "X11", "pthread", "Xxf86vm",  "Xrandr", "Xi" }
        os.rmdir("external/glfw-3.4.bin.WIN64")
        printf("linux setup")
    end

   if os.host() == "macosx" then
      enablewarnings { "all", "extra", "shadow", "cast-align", "null-dereference" }
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