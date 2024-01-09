#include "SceneEditor.hpp"

#include <iostream>

//https://vkguide.dev/docs/extra-chapter/implementing_imgui/
//https://frguthmann.github.io/posts/vulkan_imgui/
//https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html

namespace engine {

    SceneEditor::SceneEditor(Device& device,  Window& window, Renderer& renderer) 
    : device(device), window(window), renderer{renderer} {

        //seems extremely overkill
        //only modified once per launch
        imguiPool = DescriptorPool::Builder(device)
        .setMaxSets(1000)
        .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)

        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)

        .build();

        //init imgui things =========================
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = device.getInstance();
        initInfo.PhysicalDevice = device.physicalDevice;
        initInfo.Device = device.device();
        initInfo.Queue = device.graphicsQueue();
        initInfo.DescriptorPool = imguiPool->getDescriptorPool();
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.MSAASamples = device.msaaSamples;
        
        ImGui_ImplVulkan_Init(&initInfo, renderer.getSwapchainRenderPass());
        ImGui_ImplVulkan_CreateFontsTexture();
    }
    SceneEditor::~SceneEditor() {
        ImGui_ImplVulkan_RemoveTexture(viewportDescriptorSet);

        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
    }

    void SceneEditor::run(VkCommandBuffer commandBuffer) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        
        ImGui::NewFrame();

        //Viewport Window ===========================================================================================================================================
        ImGui::Begin("Viewport", false, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
        ImGui::Image((ImTextureID)viewportDescriptorSet, ImVec2(viewportExtent.width, viewportExtent.height));
        ImGui::End();

        //mess with object Window ====================================================
        ImGui::Begin("Object thingy");
        
        ImGui::End();

        //ImGui::ShowDemoWindow();
        ImGui::Render();

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void SceneEditor::configureViewport(VkImageView imageView, VkSampler sampler, VkExtent2D extent) {
        viewportDescriptorSet = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        viewportExtent = extent;
    }

    VkRenderPassCreateInfo* SceneEditor::configureRenderPass() {
        std::vector<VkSurfaceFormatKHR> availableFormats = device.getSwapChainSupport().formats;
        VkFormat swapFormat;
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapFormat = availableFormat.format;
            break;
            }
            swapFormat = availableFormats[0].format;
        }

        static std::array<VkAttachmentDescription, 2> attachments;
        //colorAttachment (Attachment 0 must be swapchain image, this is constant every time)
            attachments[0].format = swapFormat;
            attachments[0].samples = device.msaaSamples;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        //depthAttachment
            attachments[1].format = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            attachments[1].samples = device.msaaSamples;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        static VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        static VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        //for second subpass
        static VkAttachmentReference inputReference = {};
        inputReference.attachment = 1;
        inputReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        static std::array<VkSubpassDescription, 2> subpasses {};
            subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[0].colorAttachmentCount = 1;
            subpasses[0].pColorAttachments = &colorAttachmentRef;
            subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

            subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[1].colorAttachmentCount = 1;
            subpasses[1].pColorAttachments = &colorAttachmentRef;
            subpasses[1].inputAttachmentCount = 1;
            subpasses[1].pInputAttachments = &inputReference;

        static std::array<VkSubpassDependency, 3> dependency = {};
            dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency[0].dstSubpass = 0;
            dependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependency[1].srcSubpass = 0;
            dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependency[2].srcSubpass = 0;
            dependency[2].dstSubpass = 1;
            dependency[2].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependency[2].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            dependency[2].srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            dependency[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependency[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo* renderPassInfo = new VkRenderPassCreateInfo();
        renderPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo->attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo->pAttachments = attachments.data();
        renderPassInfo->subpassCount = subpasses.size();
        renderPassInfo->pSubpasses = subpasses.data();
        renderPassInfo->dependencyCount = dependency.size();
        renderPassInfo->pDependencies = dependency.data();

        return renderPassInfo;
    }
}