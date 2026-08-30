add_library(snd_core STATIC
        src/RendererInfoGui.cpp
        src/RendererInfoGui.hpp
        src/SlotMap.hpp
        src/VmaUsage.cpp
        src/Window.cpp
        src/Window.hpp
        src/Image/Image.hpp
        src/Image/KtxImage.cpp
        src/Image/KtxImage.hpp
        src/Image/StbImage.cpp
        src/Image/StbImage.hpp
        src/Renderer/DrawCommand.hpp
        src/Renderer/IRendererBackend.hpp
        src/Renderer/Renderer.hpp
        src/Renderer/Sampler.hpp
        src/Renderer/Shaders/ShaderManager.hpp
        src/Renderer/Texture.hpp
        src/Renderer/Vulkan/VulkanCommandBuffer.cpp
        src/Renderer/Vulkan/VulkanCommandBuffer.hpp
        src/Renderer/Vulkan/VulkanDevice.cpp
        src/Renderer/Vulkan/VulkanDevice.hpp
        src/Renderer/Vulkan/VulkanRenderer.cpp
        src/Renderer/Vulkan/VulkanRenderer.hpp
        src/Renderer/Vulkan/VulkanScene.hpp
        src/Renderer/Vulkan/VulkanSwapchain.cpp
        src/Renderer/Vulkan/VulkanSwapchain.hpp
        src/DeletionQueue.cpp
        src/DeletionQueue.hpp
        src/Resources/Component/Component.hpp
        src/Resources/Component/ModelComponent.hpp
        src/Resources/GltfLoader.cpp
        src/Resources/GltfLoader.hpp
        src/Resources/Material.hpp
        src/Resources/ResourceManager.cpp
        src/Resources/ResourceManager.hpp
        src/Resources/Scene.cpp
        src/Resources/Scene.hpp
        src/Resources/SceneData.hpp
        src/Resources/SceneNode.hpp
        src/Utils/FileUtils.hpp
        src/Utils/FileUtils.cpp
        src/Utils/HashUtils.hpp
        src/Utils/ImGuiUtils.hpp
        src/Utils/ImGuiUtils.cpp
        src/Utils/StringUtils.hpp
        src/Utils/StringUtils.cpp
        src/Renderer/RenderGraph/FrameGraphBuilder.hpp
        src/Renderer/RenderGraph/FrameGraphResource.cpp
        src/Renderer/RenderGraph/FrameGraphResource.hpp
        src/Renderer/RenderGraph/FrameGraphBuilder.hpp
        src/TemplateHelpers/VariantMatch.hpp
        src/TemplateHelpers/IsLegitVariant.hpp
        src/Resources/SceneData.cpp
)

target_include_directories(snd_core PUBLIC src)

target_link_libraries(snd_core PUBLIC Vulkan::Vulkan SDL3::SDL3 glm::glm VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ktx imgui tinygltf stb)

target_compile_definitions(snd_core PUBLIC VULKAN_HPP_NO_CONSTRUCTORS)
target_compile_definitions(snd_core PUBLIC VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)
target_compile_definitions(snd_core PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)

