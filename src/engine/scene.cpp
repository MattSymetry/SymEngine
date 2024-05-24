#include "scene.h"
#include <algorithm>
#include <functional>
#include <queue>
#include "cereal/archives/binary.hpp"
#include <cereal/types/array.hpp>

Scene::Scene(glm::vec4 viewport) : m_sceneGraph(0, false, nullptr, "Scene")
{
    m_sceneSize = 1; 
    m_sceneGraphNodes.push_back(&m_sceneGraph);
    float aspect = viewport.z / viewport.w;
    m_camera = Camera(glm::vec3(-2.0f, 2.0f, 2.0f), glm::vec3(0.0,0.0,0.0), 0.0f, 90.0f, aspect);
    description = {}; 
    description.mousePos = glm::ivec2(400, 400);
    description.camera_position = m_camera.getPosition();
    description.camera_target = m_camera.getTarget();
    description.viewport = viewport; 
    description.camera_roll = m_camera.getRoll(); 
    description.camera_fov = m_camera.getFov();
    description.sceneSize = m_sceneSize;
    description.backgroundColor = m_backgroundColor;
    description.sunPos = m_sunPosition; 
    description.outlineTickness = m_outlineThickness;
    description.outlineCol = m_outlineColor;
    description.showGrid = m_showGrid;
    description.AA = m_AA;

    m_viewport = viewport; 
    AddBuffer(sizeof(description), vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &description);
    AddBuffer(4, vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &frameCount);
    SetupObjects();
    updateNodeData();
    AddBuffer(sizeof(NodeData) * m_maxObjects, vk::BufferUsageFlagBits::eStorageBuffer, vk::DescriptorType::eStorageBuffer, m_nodeData.data());
    // add buffer int with selected ID
    AddBuffer(4, vk::BufferUsageFlagBits::eStorageBuffer, vk::DescriptorType::eStorageBuffer, &m_selectedObjectId, true);
}

void Scene::AddBuffer(size_t size, vk::BufferUsageFlagBits usage, vk::DescriptorType descriptorType, void *dataPtr, bool hostVisible) {
    buffers.push_back({
        size, usage, descriptorType, dataPtr, hostVisible
    });
}

void Scene::SetupObjects() {
    
}

void Scene::KeyPressed(SDL_Keycode key) {
    // check if delete key is pressed
    if (key == SDLK_DELETE) {
		SceneGraphNode* node = GetSelectedNode();
		if (node && node->getId() > 0) {
			RemoveSceneGraphNode(node);
		}
	}
    if (key == SDLK_f) {
        m_camera.LookAt(GetSelectedNode()->getTransform()->getWorldTransform()[2]);
        description.camera_target = m_camera.getTarget();
    }
}
 
void Scene::KeyboardInput(Uint8* state) {
    m_shiftPressed = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];
}
 
void Scene::MouseInput(int x, int y) {
    m_camera.Orbit(glm::vec2(x, y), (m_deltaTime / 1000.0)*m_orbitSpeed);
    description.camera_position = m_camera.getPosition(); 
}

void Scene::WheelPressed(int x, int y) {
    glm::mat4& viewMatrix = m_camera.getViewMatrix();
    glm::vec3 right(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
    glm::vec3 up(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);
    glm::vec3 translation = right * -float(x) + up * float(y);
	m_camera.Move(translation, (m_deltaTime / 1000.0)*m_cameraSpeed);
	description.camera_position = m_camera.getPosition();
    description.camera_target = m_camera.getTarget();
}

void Scene::MousePos(int x, int y) {
	description.mousePos = glm::ivec2(x, y);
}

void Scene::CtrD() {
	SceneGraphNode* node = GetSelectedNode();
    if (node && node->getId() > 0) {
        SceneGraphNode* newNode = DuplicateNode(node, node->getParent());
        insertNodeAfter(newNode, node);
        SetSelectedId(newNode->getId());
	}
}

void Scene::CtrC() {
	SceneGraphNode* node = GetSelectedNode();
    if (node && node->getId() > 0) {
		m_copyNode = *node;
	}
}

void Scene::CtrV() {
    if (m_copyNode.getId() == 9999) {
        SceneGraphNode* afterNode = GetSelectedNode();
		SceneGraphNode* newNode = DuplicateNode(&m_copyNode, m_copyNode.getParent());
        if (afterNode && afterNode->getId() > 0) {
            insertNodeAfter(newNode, afterNode);
            if (afterNode->isGroup()) {
                newNode->setParent(afterNode);
            }
        }
        else {
            updateNodeData();
        }
		SetSelectedId(newNode->getId());
	}
}

void Scene::CtrZ() {
    if (!undoStack.empty()) {
        redoStack.push(m_tmpSceneData);
        m_tmpSceneData = undoStack.top();
        RecreateScene(&m_tmpSceneData);
        undoStack.pop();
    }
}

void Scene::CtrY() {
    if (!redoStack.empty()) {
        undoStack.push(m_tmpSceneData);
        m_tmpSceneData = redoStack.top();
        RecreateScene(&m_tmpSceneData);
        redoStack.pop();
    }
}

void Scene::performAction(SceneData data) {
    if (m_isActionOngoing) {
        m_tmpSceneData = data;
    }
    else {
        m_isActionOngoing = true;
        undoStack.push(m_tmpSceneData);
        redoStack.clear();
    }
}

void Scene::endAction() {
	m_isActionOngoing = false;
}

void Scene::saveScene(std::string filename) {
    m_filename = filename;
    std::string name = filename.substr(0, filename.find_last_of("."));
    name = name.substr(name.find_last_of("/\\") + 1);
    m_sceneGraph.rename(name);
    std::ofstream os(filename, std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    m_sceneData = CreateSnapshot(false);
    archive(m_sceneData);

}

void Scene::loadScene(std::string filename) {
    m_filename = filename;
    std::ifstream is(filename, std::ios::binary);
    cereal::BinaryInputArchive archive(is);
    archive(m_sceneData);
    m_tmpSceneData = m_sceneData;
    m_sceneSize = m_sceneData.sceneSize;
    for (int i = 0; i < m_sceneSize; i++) {
        m_nodeData[i] = m_sceneData.nodeData[i];
    }
    RecreateScene();
} 

void Scene::RecreateScene(SceneData* data) {
    if (data == nullptr) {
		data = &m_sceneData;
	}
    m_tmpSceneData = *data;
    if (!m_sceneGraph.isLeaf()) {
        for (auto& child : m_sceneGraph.getChildren()) {
            RemoveSceneGraphNode(child, false);
        }
    }
	m_sceneGraphNodes.clear();
    m_sceneSize = data->sceneSize;
    m_sceneGraph = SceneGraphNode(0, false, nullptr, data->names[m_sceneSize-1]);
    m_sceneGraph.setId(0);
	m_sceneGraphNodes.push_back(&m_sceneGraph);
    m_idCounter = 0;
    if (m_sceneSize > 1 && data->nodeData[m_sceneSize - 1].data0.x > 0) {
        for (int i = 0; i < data->nodeData[m_sceneSize - 1].data0.x; i++) {
            CreateNodeFromData(data->nodeData[m_sceneSize - 1].data0.y + i, &m_sceneGraph);
        }
    }
    m_AA = data->AA;
    m_selectedObjectId = data->selectedId;
    description.AA = data->AA;
    m_backgroundColor = data->backgroundColor;
    description.backgroundColor = data->backgroundColor;
    m_outlineColor = data->outlineColor;
    description.outlineCol = data->outlineColor;
    m_outlineThickness = data->outlineThickness;
    description.outlineTickness = data->outlineThickness;
    m_showGrid = data->showGrid;
    description.showGrid = data->showGrid;
    m_sunPosition = data->sunPosition;
    description.sunPos = data->sunPosition;

    updateNodeData(false);
}

SceneGraphNode* Scene::CreateNodeFromData(int id, SceneGraphNode* parent) {
    m_idCounter++;
    SceneGraphNode* node = new SceneGraphNode(m_idCounter, false, nullptr, m_tmpSceneData.names[id]);
    node->setId(m_idCounter);
    node->setParent(parent);
    node->setData(m_tmpSceneData.nodeData[id]);
    m_sceneGraphNodes.push_back(node);
    if (m_tmpSceneData.nodeData[id].data0.x > 0) {
        for (int i = 0; i < m_tmpSceneData.nodeData[id].data0.x; i++) {
			CreateNodeFromData(m_tmpSceneData.nodeData[id].data0.y + i, node);
        }
    }
	return &m_sceneGraph; 
}

SceneData Scene::CreateSnapshot(bool saveToHistory) {
    SceneData data;
    data.names = std::vector<std::string>(m_sceneSize);
    std::vector<NodeData> nodes(m_sceneSize);
    for (int i = 0; i < m_sceneSize; i++) {
		nodes[i] = m_nodeData[i];
        data.names[i] = GetSceneGraphNode(m_nodeData[i].data0.w)->getName();
	}
    data.nodeData = nodes;
    data.sceneSize = m_sceneSize;
    data.AA = m_AA;
    data.selectedId = m_selectedObjectId;
    data.backgroundColor = m_backgroundColor;
    data.outlineColor = m_outlineColor;
    data.outlineThickness = m_outlineThickness;
    data.showGrid = m_showGrid;
    data.sunPosition = m_sunPosition;
    if (saveToHistory) {
        performAction(m_tmpSceneData);
        endAction();
    }
    m_tmpSceneData = data;
    return data;
}

SceneGraphNode* Scene::DuplicateNode(SceneGraphNode* node, SceneGraphNode* parent) {
    if (node) {
		SceneGraphNode* newNode = AddSceneGraphNode(node->getName() + " Copy");
		newNode->setParent(parent);
		newNode->changeBoolOperation(node->getBoolOperation());
		newNode->setGoop(node->getGoop());
		newNode->setColor(node->getColor());
		newNode->getTransform()->setPosition(node->getTransform()->getPosition());
		newNode->getTransform()->setRotation(node->getTransform()->getRotation());
        if (!node->isGroup()) {
			newNode->setIsGroup(false);
			GameObject* obj = new GameObject();
			Shape* shape = new Shape(Shape::createShape(node->getObject()->getComponent<Shape>()->getType()));
			shape->setShapeData(node->getObject()->getComponent<Shape>()->getStruct());
			obj->addComponent(shape);
			newNode->addObject(std::unique_ptr<GameObject>(obj));
		}
        else {
			newNode->setIsGroup(true);
            for (auto& child : node->getChildren()) {
                SceneGraphNode* newChild = DuplicateNode(child, newNode);
			}
		}
		return newNode;
	}
	return nullptr;
}

void Scene::SetSelectedId(int id) {
	m_selectedObjectId = id;
    performAction(m_tmpSceneData);
    m_tmpSceneData.selectedId = m_selectedObjectId;
}

void Scene::ClickedInViewPort() {
    if (hoverId != -1) {
        SetSelectedId(hoverId);
	}
    else {
        SetSelectedId(0);
    }
}
 
void Scene::MouseScroll(int y) {
    glm::vec3 dist = m_camera.getTarget() - m_camera.getPosition();
    glm::vec3 dir = glm::normalize(dist);
    m_camera.Move(dir * float(y) * m_camera.getScrollSpeed(), m_deltaTime / 1000.0, false);
    dist = m_camera.getTarget() - m_camera.getPosition();
    if (glm::length(dist) < 0.5f) {
	    m_camera.Move(-dir * float(y) * m_camera.getScrollSpeed(), m_deltaTime / 1000.0, false);
    }
	description.camera_position = m_camera.getPosition(); 
}

void Scene::Update(int deltaTime) {
    m_deltaTime = std::clamp(deltaTime, 0, 1000);
    frameCount ++;
    
    for (int i = 0; i < m_sceneSize; i++) {
        NodeData* data = &m_nodeData[i];
        SceneGraphNode* node = GetSceneGraphNode(data->data0.w);
        if (node) { 
            data->transform = node->getTransform()->getWorldTransform(); 
           
            if (!node->isGroup()) {
                data->object = node->getObject()->getData();
            }
            data->data0.z = node->getBoolOperation();
            data->data1.x = node->getGoop(); 
            data->color = node->getColor();
		}
	}
}
 
void Scene::UpdateViewport(glm::vec4 viewport) {
	m_viewport = viewport;
	description.viewport = viewport;
    if (viewport.z > 0.0f && viewport.w > 0.0f) {
		m_camera.setAspectRatio(viewport.z / viewport.w);
	}
}

void Scene::updateNodeData(bool saveHistory) {
    m_tmpNodeIndex = 0;
    SerializeNode(&m_sceneGraph);
	description.sceneSize = m_sceneSize;
    CreateSnapshot(saveHistory && m_sceneSize > 1);
}

void Scene::insertNodeAfter(SceneGraphNode* node, SceneGraphNode* moveBehindNode) {
    if (node == nullptr || moveBehindNode == nullptr) {
		return;
	}
	SceneGraphNode* newParent = moveBehindNode->getParent();
    if (newParent == nullptr) {
		return;
	}
    newParent->addChildAfter(node, moveBehindNode);
	updateNodeData();
}

void Scene::SerializeNode(SceneGraphNode* root) {
    if (root == nullptr) {
        return; 
    }
    std::unordered_map<SceneGraphNode*, int> nodeToIndexMapping;
    std::function<void(SceneGraphNode*)> PostOrderSerialize = [&](SceneGraphNode* node) {
        if (node == nullptr) return;

        std::map<int, std::vector<SceneGraphNode*>> levelNodes;
        std::queue<std::pair<SceneGraphNode*, int>> queue;
        queue.push({ root, 0 });

        // Normal level order but storing nodes by depth
        while (!queue.empty()) {
            auto [node, level] = queue.front();
            queue.pop();
            levelNodes[level].push_back(node);

            for (auto child : node->getChildren()) {
                queue.push({ child, level + 1 });
            }
        }

        // Iterate from the deepest level to the root level
        for (auto it = levelNodes.rbegin(); it != levelNodes.rend(); ++it) {
            for (auto node : it->second) {
                NodeData serializedNode;
                serializedNode.data0.z = (node == root) ? -1 : node->getBoolOperation();
                serializedNode.data0.y = m_tmpNodeIndex - node->getChildren().size();
                serializedNode.data0.x = (node->isGroup()) ? node->getChildren().size() : -1;
                serializedNode.data0.w = node->getId();
                serializedNode.data1.x = node->getGoop();
                serializedNode.color = node->getColor();

                if (!node->isGroup()) {
                    serializedNode.object = node->getObject()->getData();
                }
                serializedNode.transform = node->getTransform()->getWorldTransform();

                // Add the current node to the serialized list
                m_nodeData[m_tmpNodeIndex] = serializedNode;

                // Map the original tree node to its index in the serialized array
                nodeToIndexMapping[node] = m_tmpNodeIndex;
                m_tmpNodeIndex++;
            }
        }
        
    };
     
    PostOrderSerialize(root);

    // Fix the childrenStartIndex for non-leaf nodes
    for (int i = 0; i < m_sceneSize; i++) { 
		NodeData& n = m_nodeData[i];
        if (n.data0.x > 0) {
            SceneGraphNode* firstChild = GetSceneGraphNode(n.data0.w)->getChildren()[0];
            n.data0.y = nodeToIndexMapping[firstChild];
		} 
	}
}

void Scene::AddEmpty(SceneGraphNode* parent, bool isObject, Type shapeType) {
    SceneGraphNode* node = AddSceneGraphNode((isObject ? "Object" : "Group"));
    if (parent) {
        while (!parent->isGroup()) {
			parent = parent->getParent();
		}
		node->setParent(parent);
        node->getTransform()->setWorldPosition(parent->getTransform()->getPosition());
        node->getTransform()->setWorldRotation(parent->getTransform()->getRotation());
	}
	if (isObject) {
		node->setIsGroup(false);
        GameObject* obj = new GameObject();
        obj->addComponent(new Shape(Shape::createShape(shapeType)));
        node->addObject(std::unique_ptr<GameObject>(obj));
	}
    updateNodeData();
}

void Scene::AddSphere(glm::vec3 position, float radius, glm::vec3 color) {
	auto mygameObject = std::make_unique<GameObject>();
	mygameObject->getComponent<Transform>()->setPosition(position);
	Sphere sphereShape;
	sphereShape.radius = radius;
	mygameObject->addComponent(new Shape(sphereShape));
	
    SceneGraphNode* node = AddSceneGraphNode("Sphere");
    node->addObject(std::move(mygameObject));
    updateNodeData();
}

SceneGraphNode* Scene::AddSceneGraphNode(std::string name) {
    m_idCounter++;
    SceneGraphNode* node = new SceneGraphNode(m_idCounter, false, &m_sceneGraph, name + " " + std::to_string(m_idCounter));
    m_sceneGraphNodes.push_back(node);
    m_nodeData[m_sceneSize] = *node->getData();
    m_sceneSize++;
    m_selectedObjectId = m_idCounter; 
    return node; 
}

SceneGraphNode* Scene::GetSceneGraphNode(int id) {
    if (m_sceneGraphNodes[id] != nullptr) {
		return m_sceneGraphNodes[id];
	}
	return nullptr;
}

SceneGraphNode* Scene::GetSelectedNode() {
	return GetSceneGraphNode(m_selectedObjectId);
}

void Scene::setBackgroundColor(glm::vec4 color) {
	m_backgroundColor = color;
	description.backgroundColor = color;
    performAction(m_tmpSceneData);
    m_tmpSceneData.backgroundColor = color;
}

void Scene::setSunPosition(glm::vec4 pos) {
	m_sunPosition = pos;
	description.sunPos = pos;
	performAction(m_tmpSceneData);
	m_tmpSceneData.sunPosition = pos;
}

void Scene::setAA(int aa) {
	m_AA = aa;
	description.AA = aa;
	performAction(m_tmpSceneData);
	m_tmpSceneData.AA = aa;
}

void Scene::setOutlineColor(glm::vec4 color) {
	m_outlineColor = color;
	description.outlineCol = color;
	performAction(m_tmpSceneData);
	m_tmpSceneData.outlineColor = color;
}

void Scene::setOutlineThickness(float thickness) {
	m_outlineThickness = thickness;
	description.outlineTickness = thickness;
	performAction(m_tmpSceneData);
	m_tmpSceneData.outlineThickness = thickness;
}

void Scene::showGrid(int show) {
	m_showGrid = show;
	description.showGrid = show;
	performAction(m_tmpSceneData);
	m_tmpSceneData.showGrid = show;
}

void Scene::RemoveSceneGraphNode(SceneGraphNode* node, bool updateNodes) {
    m_selectedObjectId = 0;
    if (node) {
        m_sceneSize--;
        if (!node->isLeaf()) {
            for (auto& child : node->getChildren()) {
				RemoveSceneGraphNode(child, updateNodes);
			}
		}
        m_sceneGraphNodes[node->getId()] = nullptr;
        delete node;
	}
    if (updateNodes) updateNodeData();
}

void Scene::newScene() {
    if (!m_sceneGraph.isLeaf()) {
        for (auto& child : m_sceneGraph.getChildren()) {
            RemoveSceneGraphNode(child, false);
        }
    }
	m_sceneGraph = SceneGraphNode(0, false, nullptr, "Scene");
    m_sceneGraph.setId(0);
	m_sceneGraphNodes.clear();
	m_sceneGraphNodes.push_back(&m_sceneGraph);
	m_idCounter = 0;
	m_selectedObjectId = 0;
    setAA(1);
    showGrid(1);
    setBackgroundColor(glm::vec4(0.01f, 0.01f, 0.01f, 1.0f));
    setSunPosition(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    setOutlineThickness(0.0f);
    setOutlineColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    setCameraPosition(glm::vec3(-2.0f, 2.0f, 2.0f));
	m_sceneSize = 1;
	updateNodeData(false);
    undoStack = UndoStack(m_maxUndoRedo);
    redoStack = UndoStack(m_maxUndoRedo);
    m_filename = "";
}