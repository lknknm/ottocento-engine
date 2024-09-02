## How to build Ottocento Engine
Ottocento Engine is available to build on Windows, Linux and MacOS. It uses premake5 to either build a solution on windows
or a makefile for Linux and MacOS X. Generating the build is pretty straightforward.

#### Download the Source
```
git clone https://github.com/lknknm/vulkan-tutorial.git
```
## Dependencies:
- GLFW.
- VulkanSDK.
- GLM (OpenGL Mathematics).
- GLSLC shader compiler.

## How to build:
- Download and Install [Premake5](https://premake.github.io/download/) for either Linux, Windows or MacOS.
### Windows
- Download and Install [VisualStudio 2022](https://visualstudio.microsoft.com/pt-br/vs/) (Community), [JetBrains Rider](https://www.jetbrains.com/rider/), or any IDE that supports opening solution files for C++ projects.
- Download and Install the [VulkanSDK](https://vulkan.lunarg.com/#new_tab) package.
- Ottocento includes the binaries for the glfw-3.4.Win.64 version of [GLFW](https://www.glfw.org/). 
  But if you want to link it with your own installation/binaries, you can check the latest version [here](https://www.glfw.org/).
- In the project's root folder, run:
```shell
premake5 vs2022 --os=windows
```
- Open the generated `*.sln` file with your IDE of choice to run it. 
### Linux
#### VulkanSDK
- Download and Install the [VulkanSDK](https://vulkan.lunarg.com/#new_tab) Command-line utilities and run `vkcube`on the terminal
to confirm if your machine supports Vulkan:
```shell
sudo apt install vulkan-tools
```
or
```shell
sudo dnf install vulkan-tools
```
- Download and install the Vulkan Loader so Vulkan can look up the functions in the driver at runtime:
```shell
sudo apt install libvulkan-dev
```
or
```shell
sudo dnf install vulkan-loader-devel
```
- Download and install the Vulkan Validation layers and SPIR-V tools for debugging the project. 
```shell
sudo apt install vulkan-validationlayers-dev spirv-tools
```
or
```shell
sudo dnf install mesa-vulkan-devel
```

Alternatively, you can run `sudo pacman -S vulkan-devel` on Arch-Linux to install all the aforementioned Vulkan packages at once.
#### GLFW
- Install [GLFW](https://www.glfw.org/) by running either:
```shell
sudo apt install libglfw3-dev
```
or
```shell
sudo dnf install glfw-devel
```
or
```shell
sudo pacman -S glfw-wayland # glfw-x11 for X11 users
```
#### GLM (Optional — This repository already contains the glm library linked in the project)
- Install [GLM](https://glm.g-truc.net/) by running either:
```shell
sudo apt install libglm-dev
```
or
```shell
sudo dnf install glm-devel
```
or
```shell
sudo pacman -S glm
```
#### Makefile
Finally, in the project's root folder, run:
```shell
premake5 gmake --os=linux && make
```
Alternatively you can also generate the release build with:
```shell
make config=release
```

### All set!
Now you are all set to Build, Run and Contribute to the Ottocento Engine!