#pragma once

#include "deviceManager.hpp"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace renderer {

class SwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  SwapChain(Device &deviceRef, VkExtent2D windowExtent);
  SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain& operator=(const SwapChain &) = delete;

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

  void copyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkExtent2D extent, int imageIndex);

 private:
  void init();
  void createSwapChain();
  void createSyncObjects();

  VkFormat swapChainImageFormat;
  std::vector<VkImage> swapChainImages;

  VkSwapchainKHR swapchain;
  std::shared_ptr<SwapChain> oldSwapChain;

  Device &device;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};

}
