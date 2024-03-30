#pragma once
#include "../common/config.h"
#include "object.h"
#include "camera.h"
#include "vulkan/vkUtil/buffer.h"

struct SceneDescription {
    alignas(16) glm::vec3 camera_position;
    alignas(16) glm::vec3 camera_target;
    alignas(4) float camera_roll;
    alignas(4) float camera_fov;
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
    void MouseInput(int x, int y);
    void MouseScroll(int y);
    
private:
    Camera m_camera;
    int m_deltaTime;
    void AddBuffer(size_t size, vk::BufferUsageFlagBits usage, vk::DescriptorType descriptorType, void* dataPtr);
    void SetupObjects();
    void UpdateObjectData();
};
