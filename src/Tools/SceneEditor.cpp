#include "SceneEditor.hpp"

//https://vkguide.dev/docs/extra-chapter/implementing_imgui/

namespace engine {

    SceneEditor::SceneEditor(Device& device,  GLFWwindow* window, VkRenderPass renderPass) 
    : device(device), window(window), renderPass(renderPass) {

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

        //init imgui things
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = device.getInstance();
        initInfo.PhysicalDevice = device.physicalDevice;
        initInfo.Device = device.device();
        initInfo.Queue = device.graphicsQueue();
        initInfo.DescriptorPool = imguiPool->getDescriptorPool();
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.MSAASamples = device.msaaSamples;
        
        ImGui_ImplVulkan_Init(&initInfo, renderPass);

        //VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
        //Apparently this function no longer asks for a command buffer?
        ImGui_ImplVulkan_CreateFontsTexture();

    }
    SceneEditor::~SceneEditor() {
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
    }

}