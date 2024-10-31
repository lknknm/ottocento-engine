## How to build Ottocento Engine
Ottocento Engine is available to build on Windows, Linux and MacOS. It uses cmake to either build a solution on windows
or a makefile for Linux and MacOS. Generating the build is pretty straightforward.
<!-- A makefile is one option, but cmake can generate other build-system files as well. Like Ninja. Maybe the instructions should reflect this. -->

#### Download the Source
```
git clone https://github.com/lknknm/ottocento-engine.git
```
## Dependencies:
- VulkanSDK
Download and Install the [VulkanSDK](https://vulkan.lunarg.com/#new_tab) package.
<!-- Should we tell people about environment variables? -->

## How to build:
- Download and Install [cmake](https://cmake.org/download/) for either Linux, Windows or MacOS.
- From the project source directory run:
```shell
cmake -B build && cmake --build build
```
(Linux and Mac only)
Alternatively you can generate the release build with:
```shell
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

## Windows
- Download and Install [VisualStudio 2022](https://visualstudio.microsoft.com/pt-br/vs/) (Community), [JetBrains Rider](https://www.jetbrains.com/rider/), or any IDE that supports opening solution files for C++ projects.
- In the project's root folder, run:
- Open the generated `*.sln` file with your IDE of choice to run it. 
## Linux
### VulkanSDK
##### Download and Install the [VulkanSDK](https://vulkan.lunarg.com/#new_tab) Command-line utilities and run `vkcube`on the terminal to confirm if your machine supports Vulkan:
```shell
sudo apt install vulkan-tools
```
or
```shell
sudo dnf install vulkan-tools
```
#### Download and install the Vulkan Loader so Vulkan can look up the functions in the driver at runtime:
```shell
sudo apt install libvulkan-dev
```
or
```shell
sudo dnf install vulkan-loader-devel
```
#### Download and install the Vulkan Validation layers and SPIR-V tools for debugging the project. 
```shell
sudo apt install vulkan-validationlayers-dev spirv-tools
```
or
```shell
sudo dnf install mesa-vulkan-devel
```

Alternatively, you can run `sudo pacman -S vulkan-devel` on Arch-Linux to install all the aforementioned Vulkan packages at once.

## All set!
Now you are all set to Build, Run and Contribute to the Ottocento Engine!