#pragma once
#include "../common/config.h"
#include "camera.h"
#include "vulkan/vkUtil/buffer.h"
#include "SceneGraphNode.h"

struct SceneDescription {
    alignas(16) glm::vec3 camera_position;
    alignas(16) glm::vec3 camera_target;
    alignas(16) glm::vec4 viewport;
    alignas(4) float camera_roll;
    alignas(4) float camera_fov;
    alignas(4) int sceneSize;
};

class Scene {

public:
    Scene(glm::vec4 viewport);
    
    std::vector<BufferInitParams> buffers;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::vector<ObjectData> objectData;
    glm::vec4 m_viewport;
    
    SceneDescription description;

    int frameCount = 0;

    void Update(int deltaTime);
    void KeyPressed(SDL_Keycode key);
    void KeyboardInput(Uint8* state);
    void MouseInput(int x, int y);
    void MouseScroll(int y);
    void UpdateViewport(glm::vec4 viewport);

    void RemoveSceneGraphNode(SceneGraphNode* node);
    void AddEmpty(SceneGraphNode* parent = nullptr, bool isObject = false);
    void AddSphere(glm::vec3 position, float radius, glm::vec3 color);

    SceneGraphNode* GetSceneGraph() { return &m_sceneGraph; }
    int GetSelectedId() { return m_selectedObjectId; }
    void SetSelectedId(int id) { m_selectedObjectId = id; }
    SceneGraphNode* GetSceneGraphNode(int id);
    SceneGraphNode* GetSelectedNode();
    void updateNodeData();
    std::array<NodeData, 50> GetNodeData() { return m_nodeData; }
    Camera m_camera;
private:
    int m_tmpNodeIndex = 0;
    int m_deltaTime;
    std::vector<SceneGraphNode*> m_sceneGraphNodes;
    SceneGraphNode m_sceneGraph;
    int m_selectedObjectId = 0;
    int m_idCounter = 0;
    int m_sceneSize = 0;
    static const int m_maxObjects = 50;
    std::array<NodeData, m_maxObjects> m_nodeData;
    void SerializeNode(SceneGraphNode* node, int parentIndex, int index);
    void AddBuffer(size_t size, vk::BufferUsageFlagBits usage, vk::DescriptorType descriptorType, void* dataPtr);
    void SetupObjects();
    void UpdateObjectData();
    SceneGraphNode* AddSceneGraphNode(std::string name);
};
