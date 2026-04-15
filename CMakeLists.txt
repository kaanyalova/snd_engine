cmake_minimum_required(VERSION 3.31)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

project(sneed)

find_package(Vulkan REQUIRED)

include(FetchContent)
FetchContent_Declare(
        SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-3.4.0.zip
        URL_HASH SHA256=9ac2debb493e0d3e13dbd2729fb91f4bfeb00a0f4dff5e04b73cc9bac276b38d
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
        GLM
        URL https://github.com/g-truc/glm/releases/download/1.0.3/glm-1.0.3.zip
        URL_HASH SHA256=1c0a0fced9b0d87c7b7bc94e40be490cff6d4c83c25db8488d8f33754e7fdeb2
)
FetchContent_MakeAvailable(GLM)

FetchContent_Declare(
        Vma-Hpp
        URL https://github.com/YaaZ/VulkanMemoryAllocator-Hpp/releases/download/v3.3.0%2B2/VulkanMemoryAllocator-Hpp-3.3.0.tar.gz
        URL_HASH SHA256=44767b5990dd36bdc3934b48a7bda2906a5573199a12b61ef2f0de8e72f988f1
)
FetchContent_MakeAvailable(Vma-Hpp)


# github's archives are completely broken for this one
FetchContent_Declare(
    Ktx
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software
    GIT_TAG v4.4.2
    GIT_SHALLOW 1
    SOURCE_SUBDIR lib/
)
FetchContent_MakeAvailable(Ktx)


add_executable(sneed src/main.cpp
        src/Renderer.cpp
        src/Window.cpp
        src/Window.hpp
        src/Renderer.hpp
        src/Utils/FileUtils.cpp
        src/Utils/FileUtils.hpp
        src/Utils/StringUtils.cpp
        src/Utils/StringUtils.hpp
        src/Vertex.hpp
        src/Vertex.cpp
        src/VmaUsage.cpp
        src/RendererStructs.cpp
        src/RendererStructs.hpp
)

target_link_libraries(sneed PRIVATE Vulkan::Vulkan SDL3::SDL3 glm::glm VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ktx)

target_compile_definitions(sneed PRIVATE VULKAN_HPP_NO_CONSTRUCTORS)
