include(FetchContent)
FetchContent_Declare(
   vcpkg
   GIT_REPOSITORY https://github.com/microsoft/vcpkg
   GIT_TAG 0789630513565c6ea73caad01f98313cdec1b073 # 2032.08.15
)

FetchContent_MakeAvailable(vcpkg)

set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake")
