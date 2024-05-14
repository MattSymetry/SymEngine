#pragma once
#include "../common/config.h"
#include "camera.h"
#include "vulkan/vkUtil/buffer.h"
#include "SceneGraphNode.h"

struct SceneDescription {
    alignas(16) glm::ivec2 mousePos;
    alignas(16) glm::vec3 camera_position;
    alignas(16) glm::vec3 camera_target;
    alignas(16) glm::vec4 viewport;
    alignas(4) float camera_roll;
    alignas(4) float camera_fov;
    alignas(4) int sceneSize;
    alignas(16) glm::vec4 backgroundColor;
    alignas(16) glm::vec4 sunPos;
    alignas(4) float outlineTickness;
    alignas(16) glm::vec4 outlineCol;
    alignas(4) int showGrid;
    alignas(4) int AA;
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
    void WheelPressed(int x, int y);
    void MouseScroll(int y);
    void UpdateViewport(glm::vec4 viewport);

    void RemoveSceneGraphNode(SceneGraphNode* node);
    void AddEmpty(SceneGraphNode* parent = nullptr, bool isObject = false, Type shape = Type::Sphere);
    void AddSphere(glm::vec3 position, float radius, glm::vec3 color);

    SceneGraphNode* GetSceneGraph() { return &m_sceneGraph; }
    int GetSelectedId() { return m_selectedObjectId; }
    void SetSelectedId(int id) { m_selectedObjectId = id; }
    SceneGraphNode* GetSceneGraphNode(int id);
    SceneGraphNode* GetSelectedNode();
    void updateNodeData();
    std::array<NodeData, 50> GetNodeData() { return m_nodeData; }
    Camera m_camera;
    glm::vec4 getBackgroundColor() { return m_backgroundColor; }
    void setBackgroundColor(glm::vec4 color) { m_backgroundColor = color; description.backgroundColor = color; }
    glm::vec4 getSunPosition() { return m_sunPosition; }
    void setSunPosition(glm::vec4 pos) { m_sunPosition = pos; description.sunPos = pos; }
    float getOutlineThickness() { return m_outlineThickness; }
    void setOutlineThickness(float thickness) { m_outlineThickness = thickness; description.outlineTickness = thickness; }
    glm::vec4 getOutlineColor() { return m_outlineColor; }
    void setOutlineColor(glm::vec4 color) { m_outlineColor = color; description.outlineCol = color; }
    void insertNodeAfter(SceneGraphNode* node, SceneGraphNode* moveBehindNode);
    void showGrid(int show) { m_showGrid = show; description.showGrid = show; }
    int getShowGrid() { return m_showGrid; }
    void setAA(int aa) { m_AA = aa; description.AA = aa; }
    int getAA() { return m_AA; }
    void MousePos(int x, int y);
    int hoverId = -1;
    void ClickedInViewPort();
private:
    glm::vec4 m_backgroundColor = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);
    glm::vec4 m_sunPosition = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_outlineThickness = 0.0f;
    glm::vec4 m_outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    bool m_shiftPressed = false;
    int m_tmpNodeIndex = 0;
    int m_deltaTime;
    std::vector<SceneGraphNode*> m_sceneGraphNodes;
    SceneGraphNode m_sceneGraph;
    int m_selectedObjectId = 0;
    int m_idCounter = 0;
    int m_sceneSize = 0;
    int m_showGrid = 1;
    int m_AA = 1;
    static const int m_maxObjects = 50;
    std::array<NodeData, m_maxObjects> m_nodeData;
    void SerializeNode(SceneGraphNode* node);
    void AddBuffer(size_t size, vk::BufferUsageFlagBits usage, vk::DescriptorType descriptorType, void* dataPtr);
    void SetupObjects();
    void UpdateObjectData();
    SceneGraphNode* AddSceneGraphNode(std::string name);
};
