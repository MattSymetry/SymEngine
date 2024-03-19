#pragma once
#include "../common/config.h"
#include "object.h"
#include "camera.h"
#include "vulkan/vkUtil/buffer.h"

struct SceneDescription {
    alignas(16) glm::vec3 camera_right;
    alignas(16) glm::vec3 camera_forward;
    alignas(16) glm::vec3 camera_up;
    alignas(16) glm::vec3 camera_position;
    alignas(4) int sphereCount;
    alignas(4) int boxCount;
    alignas(4) int coneCount;
};

class Scene {

public:
    Scene();
    
    std::vector<BufferInitParams> buffers;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::vector<ObjectData> objectData;
    
    SceneDescription description;

    int frameCount = 0;

    void Update(int deltaTime);
    void KeyPressed(SDL_Keycode key);
    void KeyboardInput(Uint8* state);
    
private:
    Camera m_camera;
    int m_deltaTime;
    void AddBuffer(size_t size, vk::BufferUsageFlagBits usage, vk::DescriptorType descriptorType, void* dataPtr);
    void SetupObjects();
    void UpdateObjectData();
};
