include(FetchContent)
FetchContent_Declare(
   vcpkg
   GIT_REPOSITORY https://github.com/microsoft/vcpkg
   GIT_TAG 62efe42f53b1886a20cbeb22ee9a27736d20f149
)

FetchContent_MakeAvailable(vcpkg)

set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
