#pragma once

#include "windowManager.hpp" 
#include "deviceManager.hpp"
#include "swapchainManager.hpp"
#include "RenderPass.hpp"
#include "descriptorManager.hpp"
#include "bufferManager.hpp"
#include "Utils.hpp"
//#include "ECS/Importer.hpp"

#include <memory>
#include <vector>
#include <cassert>
#include <map>
#include <iostream>

namespace renderer {

enum class DefinedRenderPasses {
   Primary 
};


//maybe I shouldnt generalize unless I need to.
//for use with POD structs
template<typename T>
struct Gallery {
    T& getExhibit(std::string path) {
		assert(exhibits.count(path) && "Trying to retrieve a nonexistant Exhibit!");
		return exhibits.at(path).first;
    }

    T& createExhibit(std::string path, T exhibit) {
		if(exhibits.count(path)) {
			exhibits[path].second++;
			std::cout << "exhibit reused :)\n";
		} 
		else exhibits.emplace(path, std::pair{std::move(exhibit), 1});
		return exhibits.at(path).first;
	}

    void removeExhibit(std::string path) {
		assert(exhibits.count(path) && "Trying to remove a nonexistant Exhibit!");
		auto& image = exhibits.at(path);
		--image.second;
		if(image.second <= 0) {
			exhibits.erase(path);
			std::cout << "exhibit destroyed\n";
		}
    }

    private:
    std::unordered_map<std::string, std::pair<T, int>> exhibits = {};
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

	VkSampler defaultSampler;

	Device device;
	Window& window;

	VkRenderPassCreateInfo* primaryPassInfo;
	std::unique_ptr<RenderPass> primaryRenderPass;

	std::shared_ptr<DescriptorPool> globalPool;
	std::unique_ptr<DescriptorSetLayout> globalSetLayout;
	std::vector<std::unique_ptr<Buffer>> uboBuffers{SwapChain::MAX_FRAMES_IN_FLIGHT};
	std::vector<VkDescriptorSet> globalDescriptorSets{SwapChain::MAX_FRAMES_IN_FLIGHT};

	Gallery<AllocatedImage> imageGallery{};
	
    private:
	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapChain();
	void resizeRenderPasses(); 
	std::unique_ptr<SwapChain> swapchain;
	std::vector<VkCommandBuffer> commandBuffers;

	// Image index and frame index are different things!!!!!
	uint32_t currentImageIndex;
	int currentFrameIndex{0};
	bool isFrameStarted = false;

};


// =============================================================================================================================================================

}
