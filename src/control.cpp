#include "engineControl.h"
#include "keyMap.h"

#include "Pipeline/windowManager.hpp"
#include "Pipeline/Renderer.hpp"
#include "engine.hpp"
#include "ECS/AssetManager.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <chrono>

static Window* window = nullptr;  
static renderer::Renderer* Renderer = nullptr;
static ECS::AssetSystem* assetSystem = nullptr;
static engine::engine* Engine = nullptr;

void createEngine(int width, int height, HWND handle) {
    window = new Window_win{width, height, handle};
    Renderer = new renderer::Renderer{*window};
    assetSystem = new ECS::AssetSystem;
    Engine = new engine::engine{*Renderer, *assetSystem};

    //TODO: refactor =====================================================
	ECS::Entity viewportEntity = assetSystem->CreateEntity();
	ECS::Transform& viewportTransform = assetSystem->AddComponent(viewportEntity, ECS::Transform{glm::vec3(0.0f, -3.5f, -12.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f)});
	ECS::Camera& viewportCamera = assetSystem->AddComponent(viewportEntity, ECS::Camera{0.1f, 5000.0f});

    // moveInPlaneXZ(window, viewportTransform, dt, 2.5f, 3.0f);
    
    viewportCamera.viewMatrix = engine::setViewYXZ(viewportTransform.translation, viewportTransform.rotation);            
    float aspect = Renderer->getAspectRatio();
    viewportCamera.projectionMatrix = engine::setPerspectiveProjection(glm::radians(50.0f), aspect, viewportCamera.nearPlane, viewportCamera.farPlane);
    viewportCamera.inverseViewMatrix = glm::inverse(viewportCamera.viewMatrix);

    Engine->viewerObject = viewportEntity;
    //=========================================================================
}

void resize(int width, int height) {
    window->resizeWindow(width, height);
}

void destroyEngine() {
    delete Engine;
    delete assetSystem;
    delete Renderer;
    delete window;
}

bool runFrame() {

    static auto currentTime = std::chrono::high_resolution_clock::now();
    if(!Renderer->window.shouldClose()) {
        //glfwPollEvents();

        //get passed time
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime-currentTime).count();
        currentTime = newTime;

        Engine->updateGameState(frameTime, *window);
        Engine->renderGameState(assetSystem->GetComponent<ECS::Camera>(Engine->viewerObject));
        vkDeviceWaitIdle(Renderer->device.device());
        return true;
    }
    else {
        return false;
    }
}

void buttonState(Key key, bool state) {
    if(state) {
        window->setKeyDown(key);
    }
    else if (!state) {
        window->setKeyUp(key);
    }
}

#ifdef Entity
#undef Entity
#endif

void unloadData(std::string fileName) {
    Engine->saveDataManager->unloadData(fileName, *assetSystem);
}

void loadCollection(std::string fileName) {
    Engine->saveDataManager->loadData(fileName, *assetSystem);
}