#include "scene.h"
#include <algorithm>
#include <functional>
#include <queue>

Scene::Scene(glm::vec4 viewport) : m_sceneGraph(0, false, nullptr, "Scene")
{
    m_sceneSize = 1; 
    m_sceneGraphNodes.push_back(&m_sceneGraph);
    float aspect = viewport.z / viewport.w;
    m_camera = Camera(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(0.0,0.0,0.0), 0.0f, 90.0f, aspect);
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

void Scene::ClickedInViewPort() {
    if (hoverId != -1) {
		m_selectedObjectId = hoverId;
	}
    else {
        m_selectedObjectId = 0;
    }
}
 
void Scene::MouseScroll(int y) {
    glm::vec3 dir = glm::normalize(m_camera.getTarget() - m_camera.getPosition());
	m_camera.Move(dir * float(y) * m_camera.getScrollSpeed(), m_deltaTime / 1000.0);
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

void Scene::updateNodeData() {
    m_tmpNodeIndex = 0;
    SerializeNode(&m_sceneGraph);
	description.sceneSize = m_sceneSize;
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

void Scene::RemoveSceneGraphNode(SceneGraphNode* node) {
    m_selectedObjectId = 0;
    if (node) {
        m_sceneSize--;
        if (!node->isLeaf()) {
            for (auto& child : node->getChildren()) {
				RemoveSceneGraphNode(child);
			}
		}
        m_sceneGraphNodes[node->getId()] = nullptr;
        delete node;
	}
    updateNodeData();
}