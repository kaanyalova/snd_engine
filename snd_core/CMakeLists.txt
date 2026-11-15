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
        TemplateHelpers/VariantMatch.hpp
        TemplateHelpers/IsLegitVariant.hpp
        Resources/Scene/SceneData.cpp
        Renderer/Vulkan/VulkanScene.cpp
        Image/ImageUtils.cpp
        Image/ImageUtils.hpp
        Renderer/Vulkan/VulkanImage.cpp
        Renderer/Vulkan/VulkanImage.hpp
        Renderer/Vulkan/VulkanSampler.cpp
        Renderer/Vulkan/VulkanSampler.hpp
        Image/Image.cpp
        Resources/Scene/SceneNode.cpp
        Resources/Scene/SceneTestData.cpp
        Resources/Scene/SceneTestData.hpp
        Resources/Scene/BakedScene.cpp
        Resources/Scene/BakedScene.hpp
        Renderer/Vulkan/VulkanBuffer.cpp
        Renderer/Vulkan/VulkanBuffer.hpp

        Resources/Component/CameraComponent.cpp
        Resources/Component/CameraComponent.hpp
        Renderer/RenderGraph/RenderPass.cpp
        Renderer/RenderGraph/RenderPass.hpp
        Renderer/RenderGraph/RenderGraph.cpp
        Renderer/RenderGraph/RenderGraph.hpp
        Renderer/RenderGraphImpl/TestRenderPass.cpp
        Renderer/RenderGraphImpl/TestRenderPass.hpp
)

# include dir is the parent so includes read "snd_core/Window.hpp"
target_include_directories(snd_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(snd_core PUBLIC Vulkan::Vulkan SDL3::SDL3 glm::glm VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp ktx imgui tinygltf stb)

target_compile_definitions(snd_core PUBLIC VULKAN_HPP_NO_CONSTRUCTORS)
target_compile_definitions(snd_core PUBLIC VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS)
# vma-hpp-raii doesn't like the spaceship operator now, idk what changed
target_compile_definitions(snd_core PUBLIC VULKAN_HPP_NO_SPACESHIP_OPERATOR)
target_compile_definitions(snd_core PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
