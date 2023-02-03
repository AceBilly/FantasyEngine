cmake_minimum_required(VERSION 3.24)
project(Directx12Render)

set(CMAKE_CXX_STANDARD 23)

# DirectX header and lib directories
include_directories($ENV{WindowsSdkDir\\Include\\$ENV{WindowsSDKVersion}\\um\\})
link_directories($ENV{WindowsSdkDir\\Lib\\$ENV{WindowsSDKVersion}\\um\\arch\\})

add_executable(Directx12Render WIN32 main.cpp engine/modules/error.ixx engine/modules/D3D12Core.ixx engine/modules/render.ixx engine/modules/window.ixx engine/modules/common.ixx)

target_link_libraries(Directx12Render D3d12.lib)

