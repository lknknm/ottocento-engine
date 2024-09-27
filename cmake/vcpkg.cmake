include(FetchContent)
FetchContent_Declare(
   vcpkg
   GIT_REPOSITORY https://github.com/microsoft/vcpkg
   GIT_TAG 3508985146f1b1d248c67ead13f8f54be5b4f5da
)

FetchContent_MakeAvailable(vcpkg)

set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
