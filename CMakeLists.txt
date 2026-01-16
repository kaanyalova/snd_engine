cmake_minimum_required(VERSION 3.31)

# Enable C++ modules support
SET(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
SET(CMAKE_CXX_MODULE_STD 1)

set(CMAKE_CXX_STANDARD 26)

project(sneed)

find_package(Vulkan REQUIRED)

# set up cpp modules for vulkan
add_library(VulkanHppModule)

target_compile_definitions(VulkanHppModule
        PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_sources(VulkanHppModule PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${Vulkan_INCLUDE_DIR}
        FILES
        ${Vulkan_INCLUDE_DIR}/vulkan/vulkan_video.cppm ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
target_compile_features(VulkanHppModule PUBLIC cxx_std_20)
target_link_libraries(VulkanHppModule PUBLIC Vulkan::Vulkan)


include(FetchContent)
FetchContent_Declare(
        SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-3.4.0.zip
        URL_HASH SHA256=9ac2debb493e0d3e13dbd2729fb91f4bfeb00a0f4dff5e04b73cc9bac276b38d
)
FetchContent_MakeAvailable(SDL3)


add_executable(sneed src/main.cpp
        src/Renderer.cpp
        src/Window.cppm
        src/Window.cpp)

target_sources(sneed
        PUBLIC
        FILE_SET modules TYPE CXX_MODULES FILES
        src/Renderer.cppm
        src/Window.cppm
)


target_link_libraries(sneed PRIVATE Vulkan::Vulkan VulkanHppModule SDL3::SDL3)