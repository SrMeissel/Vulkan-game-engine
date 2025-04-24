#pragma once

#include "deviceManager.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace renderer {

class SwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  SwapChain(Device &deviceRef, const VkRenderPassCreateInfo& info, VkExtent2D windowExtent);
  SwapChain(Device &deviceRef, const VkRenderPassCreateInfo& info, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain& operator=(const SwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
  VkRenderPass& getRenderPass() { return renderPass; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }

  VkExtent2D windowExtent;
  VkExtent2D swapChainExtent;
  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  bool compareSwapChain(const SwapChain& swapChain) const {
    return swapChain.swapChainImageFormat == swapChainImageFormat;
  }
  // Helper functions
  static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  static VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

 private:
  void init(const VkRenderPassCreateInfo& info);
  void createSwapChain();
  void createImageViews();
  void createRenderPass(const VkRenderPassCreateInfo& info);
  void createFramebuffers();
  void createSyncObjects();

  VkFormat swapChainImageFormat;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  Device &device;

  VkSwapchainKHR swapchain;
  std::shared_ptr<SwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};

}
