#pragma once

#include "windowManager.hpp" 
#include "deviceManager.hpp"
#include "swapchainManager.hpp"
#include "RenderPass.hpp"
#include "descriptorManager.hpp"
#include "bufferManager.hpp"
#include "Utils.hpp"
#include "ECS/Importer.hpp"

#include <memory>
#include <vector>
#include <cassert>
#include <map>

namespace renderer {

enum class DefinedRenderPasses {
   Primary 
};

class Renderer {
    public:
	

	Renderer(Window& window);
	~Renderer();

	Renderer(const Renderer &) = delete;
	Renderer &operator=(const Renderer &) = delete;

	float getAspectRatio() const {return swapchain->extentAspectRatio(); }
	bool isFrameInProgress() const { return isFrameStarted; }

	VkCommandBuffer getCurrentCommandBuffer() const {
	    assert(isFrameStarted && "Cannot get frame buffer when frame is in progress!");
	    return commandBuffers[currentFrameIndex];    
	}

	// FRAME INDEX IS DIFFERENT THAN IMAGE INDEX !!!!!
	int getFrameIndex() const {
	    assert(isFrameStarted && "Cannot get frame index while frame is not in progress!");
	    return currentFrameIndex;
	}
	uint32_t getCurrentImageIndex() const {return currentImageIndex; }

	VkCommandBuffer beginFrame();
	void endFrame();

	void beginRenderPass(VkCommandBuffer commandBuffer, DefinedRenderPasses pass);
	void endRenderPass(VkCommandBuffer commandBuffer);

	VkSampler& getDefaultSampler() {return defaultSampler; };

	Device device;
	Window& window;

	VkRenderPassCreateInfo* primaryPassInfo;
	std::unique_ptr<RenderPass> primaryRenderPass;

	std::shared_ptr<DescriptorPool> globalPool;
	std::unique_ptr<DescriptorSetLayout> globalSetLayout;
	std::vector<std::unique_ptr<Buffer>> uboBuffers{SwapChain::MAX_FRAMES_IN_FLIGHT};
	std::vector<VkDescriptorSet> globalDescriptorSets{SwapChain::MAX_FRAMES_IN_FLIGHT};
	
    private:
	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapChain();
	void resizeRenderPasses() {}; // if renderpass uses window extent that needs to be resizes with the window

	std::unique_ptr<SwapChain> swapchain;
	std::vector<VkCommandBuffer> commandBuffers;

	// Image index and frame index are different things!!!!!
	uint32_t currentImageIndex;
	int currentFrameIndex{0};
	bool isFrameStarted = false;

	VkSampler defaultSampler;
};


// =============================================================================================================================================================

//the gallery serves as a way to store all images by reference path, may not be used.
struct Gallery {
    Gallery(Device& device) : device{device} {
	deleter = [&device](AllocatedImage* image) {
	    vkDestroyImageView(device.device(), image->imageView, nullptr);
	    vkDestroyImage(device.device(), image->image, nullptr);
	    vkFreeMemory(device.device(), image->memory, nullptr);
	    delete image;
	};
    }

    AllocatedImage* getImage(std::string path) {
	return images[path].first.get();
    }

    void createImage(std::string path) {
	if(images.count(path)) images[path].second++; 
	else images[path] = {std::make_unique<AllocatedImage>(Importer::loadJPGImage(path, device, VK_FORMAT_R8G8B8A8_SRGB)), 1};
    }

    void removeImage(std::string path) {
	assert(images.count(path) && "Trying to remove a nonexistant Exhibit!");
	auto& image = images[path];
	--image.second;
	if(image.second <= 0)   images.erase(path);
    }

    private:
    Device& device;
    std::function<void(AllocatedImage*)> deleter;
    std::map<std::string, std::pair<std::unique_ptr<AllocatedImage, decltype(deleter)>, int>> images;
};

struct Exhibit {
    std::string path;

    Exhibit(std::string path, Gallery& gallery) : gallery{gallery}, path{path} {
	gallery.createImage(path);
    }
    ~Exhibit() {
	gallery.removeImage(path);
    }

    private:
    Gallery& gallery;
};


}
