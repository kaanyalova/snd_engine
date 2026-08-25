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
    GIT_TAG 79e071abf4e02c5b43a6a5b57b2db27137e1cef3
    GIT_SHALLOW 1
    SOURCE_SUBDIR lib/
)
FetchContent_MakeAvailable(Ktx)


# imgui doesn't have a CMakeLists.txt file so i have to suffer though this shit
FetchContent_Declare(
    imgui_src
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.8.zip
    URL_HASH SHA256=27765c56ab27ce47472d0bea43cf1e3301c726362ce585e99a059e3b37616870
)

FetchContent_MakeAvailable(imgui_src)

set(IMGUI_DIR ${imgui_src_SOURCE_DIR})

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp

    # backend files
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
)

add_library(
    imgui STATIC
    ${IMGUI_SOURCES}
)

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PRIVATE Vulkan::Vulkan SDL3::SDL3)

FetchContent_Declare(
    tinygltf
    URL https://github.com/syoyo/tinygltf/archive/refs/tags/v3.0.0.zip
    URL_HASH SHA256=6e2583eb75d113446c593c249d6ac0ddb00662174935d4819317da2223934884
)


set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)

FetchContent_MakeAvailable(tinygltf)


FetchContent_Declare(
    stb_src
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG 08c7e1f0d8a2b9f3e5c6d4b1e5f3c6d4b1e5f3c6
    GIT_SHALLOW 1
)

add_library(
    stb INTERFACE
    ${stb_src_SOURCE_DIR}
)


FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(sneed
    src/main.cpp
    src/RendererInfoGui.cpp
    src/RendererInfoGui.hpp
    src/SlotMap.hpp
    src/VmaUsage.cpp
    src/Window.cpp
    src/Window.hpp
    src/Image/KtxImage.cpp
    src/Image/KtxImage.hpp
    src/Renderer/Vulkan/VulkanCommandBuffer.cpp
    src/Renderer/Vulkan/VulkanCommandBuffer.hpp
    src/DeletionQueue.cpp
    src/DeletionQueue.hpp
    src/Renderer/Vulkan/VulkanDevice.cpp
    src/Renderer/Vulkan/VulkanDevice.hpp
    src/Renderer/Vulkan/VulkanSwapchain.cpp
    src/Renderer/Vulkan/VulkanSwapchain.hpp
    src/Renderer/Vulkan/VulkanRenderer.cpp
    src/Renderer/Vulkan/VulkanRenderer.hpp
    src/Resources/GltfLoader.cpp
    src/Resources/GltfLoader.hpp
    src/Resources/Material.hpp
    src/Resources/ResourceManager.cpp
    src/Resources/SceneData.hpp
    src/Utils/FileUtils.hpp
    src/Utils/FileUtils.cpp
    src/Utils/ImGuiUtils.hpp
    src/Utils/ImGuiUtils.cpp
    src/Utils/StringUtils.hpp
    src/Utils/StringUtils.cpp
)

target_link_libraries(sneed PRIVATE Vulkan::Vulkan SDL3::SDL3 glm::glm VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ktx imgui tinygltf stb)

target_compile_definitions(sneed PRIVATE VULKAN_HPP_NO_CONSTRUCTORS)
target_compile_definitions(sneed PRIVATE VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)

target_compile_definitions(sneed PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)


