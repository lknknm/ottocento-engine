include(FetchContent)
FetchContent_Declare(
   vcpkg
   GIT_REPOSITORY https://github.com/microsoft/vcpkg
   GIT_TAG 26abb5b33f976246a5f2cdc45cdd073d51caff06
)

FetchContent_MakeAvailable(vcpkg)

set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
