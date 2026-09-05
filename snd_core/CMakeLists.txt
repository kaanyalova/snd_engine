add_library(snd_core STATIC
        RendererInfoGui.cpp
        RendererInfoGui.hpp
        SlotMap.hpp
        VmaUsage.cpp
        Window.cpp
        Window.hpp
        Image/Image.hpp
        Image/KtxImage.cpp
        Image/KtxImage.hpp
        Image/StbImage.cpp
        Image/StbImage.hpp
        Renderer/DrawCommand.hpp
        Renderer/IRendererBackend.hpp
        Renderer/Renderer.hpp
        Renderer/Sampler.hpp
        Renderer/Shaders/ShaderManager.hpp
        Renderer/Texture.hpp
        Renderer/Vulkan/VulkanCommandBuffer.cpp
        Renderer/Vulkan/VulkanCommandBuffer.hpp
        Renderer/Vulkan/VulkanDevice.cpp
        Renderer/Vulkan/VulkanDevice.hpp
        Renderer/Vulkan/VulkanRenderer.cpp
        Renderer/Vulkan/VulkanRenderer.hpp
        Renderer/Vulkan/VulkanScene.hpp
        Renderer/Vulkan/VulkanSwapchain.cpp
        Renderer/Vulkan/VulkanSwapchain.hpp
        DeletionQueue.cpp
        DeletionQueue.hpp
        Resources/Component/Component.hpp
        Resources/Component/ModelComponent.hpp
        Resources/GltfLoader.cpp
        Resources/GltfLoader.hpp
        Resources/Material.hpp
        Resources/ResourceManager.cpp
        Resources/ResourceManager.hpp
        Resources/Scene/Scene.cpp
        Resources/Scene/Scene.hpp
        Resources/Scene/SceneData.hpp
        Resources/Scene/SceneNode.hpp
        Utils/FileUtils.hpp
        Utils/FileUtils.cpp
        Utils/HashUtils.hpp
        Utils/ImGuiUtils.hpp
        Utils/ImGuiUtils.cpp
        Utils/StringUtils.hpp
        Utils/StringUtils.cpp
        Renderer/RenderGraph/FrameGraphBuilder.hpp
        Renderer/RenderGraph/FrameGraphResource.cpp
        Renderer/RenderGraph/FrameGraphResource.hpp
        TemplateHelpers/VariantMatch.hpp
        TemplateHelpers/IsLegitVariant.hpp
        Resources/Scene/SceneData.cpp
        Resources/Scene/ModelBuilder.cpp
        Resources/Scene/ModelBuilder.hpp
)

# include dir is the parent so includes read "snd_core/Window.hpp"
target_include_directories(snd_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(snd_core PUBLIC Vulkan::Vulkan SDL3::SDL3 glm::glm VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ktx imgui tinygltf stb)

target_compile_definitions(snd_core PUBLIC VULKAN_HPP_NO_CONSTRUCTORS)
target_compile_definitions(snd_core PUBLIC VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)
target_compile_definitions(snd_core PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
