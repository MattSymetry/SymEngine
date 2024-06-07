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
    m_camera = Camera(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0,0.0,0.0), 0.0f, 90.0f, aspect);
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

    InitShapes();

    m_viewport = viewport; 
    AddBuffer(sizeof(description), vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &description);
    AddBuffer(4, vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &frameCount);
    SetupObjects();
    updateNodeData();
    AddBuffer(sizeof(NodeData) * m_maxObjects, vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, m_nodeData.data());
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

void Scene::InitShapes() {
    m_shapes[Type::Sphere] = std::vector<ShaderShape>();
    m_shapes[Type::Box] = std::vector<ShaderShape>();
    m_shapes[Type::Cone] = std::vector<ShaderShape>();
	AddShape("sdSphere", R"(
float sdSphere( vec3 position, float radius )
{
	//float t = time / 2.0;
	//radius += 0.008*sin(t + position.x*50)+0.008*sin(t + position.y*50)+0.008*sin(t + position.z*50);
    return length(position)-radius; 
}
)", Type::Sphere);

	AddShape("sdRoundBox", R"(
float sdRoundBox( vec3 p, vec3 b, float r )
{
    vec3 q = abs(p) - b + r;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}
)", Type::Box);

	AddShape("sdCone", R"(
float sdCone( vec3 p, float h, float ra, float rb)
{
    vec3 a = vec3(0.0,h/2.0,0.0);
    vec3 b = vec3(0.0,-h/2.0,0.0);
    float rba  = rb-ra;
    float baba = dot(b-a,b-a);
    float papa = dot(p-a,p-a);
    float paba = dot(p-a,b-a)/baba;

    float x = sqrt( papa - paba*paba*baba );

    float cax = max(0.0,x-((paba<0.5)?ra:rb));
    float cay = abs(paba-0.5)-0.5;

    float k = rba*rba + baba;
    float f = clamp( (rba*(x-ra)+paba*baba)/k, 0.0, 1.0 );

    float cbx = x-ra - f*rba;
    float cby = paba - f;
    
    float s = (cbx < 0.0 && cay < 0.0) ? -1.0 : 1.0;
    
    return s*sqrt( min(cax*cax + cay*cay*baba,
                       cbx*cbx + cby*cby*baba) );
}
)", Type::Cone);
    AddShape("sdCylinder", R"(
float sdCylinder( vec3 p, float h, float r )
{
    vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(r,h);
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}
)", Type::Cylinder);
    AddShape("sdPyramid", R"(
    float sdPyramid( in vec3 p, in float h, in float w )
{
    float m2 = h*h + 0.25;
    // Adjust xz coordinates by the base width
    p.xz /= w;
    p.xz = abs(p.xz);
    p.xz = (p.z > p.x) ? p.zx : p.xz;
    p.xz -= 0.5;
    vec3 q = vec3(p.z, h*p.y - 0.5*p.x, h*p.x + 0.5*p.y);
    float s = max(-q.x, 0.0);
    float t = clamp((q.y - 0.5*q.x) / (m2 + 0.25), 0.0, 1.0);
    float a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
    float b = m2 * (q.x + 0.5 * t) * (q.x + 0.5 * t) + (q.y - m2 * t) * (q.y - m2 * t);
    float d2 = max(-q.y, q.x * m2 + q.y * 0.5) < 0.0 ? 0.0 : min(a, b);
    return sqrt((d2 + q.z * q.z) / m2) * sign(max(q.z, -p.y)) * w;
}
)", Type::Pyramid);
    AddShape("sdTorus", R"(
float sdTorus( vec3 p, float ri, float ro )
{
	vec2 t = vec2(length(p.xz)-ri,p.y);
	return length(t)-ro;
}
)", Type::Torus);
}

void Scene::AddShape(std::string name, std::string code, Type type) {
    m_shapes[type].push_back(ShaderShape(name, code));
}

float Scene::getShapeIdByName(std::string name, Type type) {
    for (int i = 0; i < m_shapes[type].size(); i++) {
		if (m_shapes[type][i].name == name) {
			return m_shapes[type][i].id;
		}
	}
    return -1.0f;
}

std::string Scene::getShapeNameById(float id, Type type) {
	for (int i = 0; i < m_shapes[type].size(); i++) {
		if (m_shapes[type][i].id == id) {
			return m_shapes[type][i].name;
		}
	}
	return "";
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
        setCameraTarget(GetSelectedNode()->getTransform()->getWorldTransform()[2]);
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
    if (m_sceneSize >= m_maxObjects) { return; }
	SceneGraphNode* node = GetSelectedNode();
    if (node && node->getId() > 0) {
        SceneGraphNode* newNode = DuplicateNode(node, node->getParent());
        if (newNode == nullptr) {return;}
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
    if (m_sceneSize >= m_maxObjects) { return; }
    if (m_copyNode.getId() == 9999) {
        SceneGraphNode* afterNode = GetSelectedNode();
		SceneGraphNode* newNode = DuplicateNode(&m_copyNode, m_copyNode.getParent());
        if (newNode == nullptr) { return; }
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

std::vector<char> LoadSymResource(UINT resourceID) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceID), TEXT("SYM"));
    if (hRes == NULL) {
        std::cerr << "Failed to find resource." << std::endl;
        return std::vector<char>();
    }
    HGLOBAL hResData = LoadResource(NULL, hRes);
    if (hResData == NULL) {
        std::cerr << "Failed to load resource." << std::endl;
        return std::vector<char>();
    }
    DWORD dataSize = SizeofResource(NULL, hRes);
    void* pResData = LockResource(hResData);
    if (pResData == NULL) {
        std::cerr << "Failed to lock resource." << std::endl;
        return std::vector<char>();
    }
    return std::vector<char>(static_cast<char*>(pResData), static_cast<char*>(pResData) + dataSize);
}

// Function to load scene data from .sym resource
template <typename T>
void LoadSceneDataFromResource(UINT resourceID, T& sceneData) {
    std::vector<char> symData = LoadSymResource(resourceID);
    if (symData.empty()) {
        std::cerr << "Failed to load .sym resource data." << std::endl;
        return;
    }

    std::istringstream is(std::string(symData.begin(), symData.end()), std::ios::binary);
    cereal::BinaryInputArchive archive(is);
    archive(sceneData);
}

void Scene::loadScene(std::string filename) {
    m_filename = filename;
    std::ifstream is(filename, std::ios::binary);
    cereal::BinaryInputArchive archive(is);
    archive(m_sceneData);
    m_sceneSize = m_sceneData.sceneSize;
    for (int i = 0; i < m_sceneSize; i++) {
        m_nodeData[i] = m_sceneData.nodeData[i];
    }
    RecreateScene();
    m_tmpSceneData = m_sceneData;
    undoStack = UndoStack(m_maxUndoRedo);
    redoStack = UndoStack(m_maxUndoRedo);
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
    m_shapes = data->shaderShapes;
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
    if (!node->isGroup()) {
        Shape* shape = node->getObject()->getComponent<Shape>();
        shape->setShaderName(getShapeNameById(m_tmpSceneData.nodeData[id].object[1][2], shape->getType()));
    }
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
    data.shaderShapes = m_shapes;
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
        if (newNode == nullptr) {return nullptr;}
		newNode->setParent(parent);
		newNode->changeBoolOperation(node->getBoolOperation());
		newNode->setGoop(node->getGoop());
        newNode->setColorGoop(node->getColorGoop());
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
            newNode->setMirrorX(node->getMirrorX());
            newNode->setMirrorY(node->getMirrorY());
            newNode->setMirrorZ(node->getMirrorZ());
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
                data->object[2][0] = node->getMirrorX();
                data->object[2][1] = node->getMirrorY();
                data->object[2][2] = node->getMirrorZ();
            }
            data->data0.z = node->getBoolOperation();
            data->data1.x = node->getGoop();
            data->data1.y = node->getColorGoop();
            data->color = node->getColor();
		}
	}
}
 
void Scene::UpdateViewport(glm::vec4 viewport, float aspectRatio) {
	m_viewport = viewport;
	description.viewport = viewport;
	m_camera.setAspectRatio(aspectRatio);
}

void Scene::updateNodeData(bool saveHistory) {
    m_tmpNodeIndex = 0;
    SerializeNode(&m_sceneGraph);
	description.sceneSize = m_sceneSize;
    CreateSnapshot(saveHistory && m_sceneSize > 1);
    needsRecompilation = true;
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
                serializedNode.data1.y = node->getColorGoop();
                serializedNode.color = node->getColor();

                if (!node->isGroup()) {
                    serializedNode.object = node->getObject()->getData();
                    serializedNode.object[2][0] = node->getMirrorX();
                    serializedNode.object[2][1] = node->getMirrorY();
                    serializedNode.object[2][2] = node->getMirrorZ();
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
    if (node == nullptr) {return;}
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

SceneGraphNode* Scene::AddSceneGraphNode(std::string name) {
    if (m_sceneSize >= m_maxObjects) {return nullptr;}
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
    if (m_selectedObjectId >= m_sceneGraphNodes.size()) {
        m_selectedObjectId = 0;
    }
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

std::string Scene::mirrirShader(std::string node, NodeData nodeData) {
    std::string str = "tmpPos = pos;\n";
    std::string parentRot = "Rotate(radians(" + node + ".transform[1].x), radians(" + node + ".transform[1].y), radians(" + node + ".transform[1].z))";
    if (nodeData.object[2][0] > 0.1f) {
		str += "tmpPos = Reflect(tmpPos, vec3(1.0,0.0,0.0), " + node + ".transform[2].xyz, " + parentRot + "); \n";
	}
    if (nodeData.object[2][1] > 0.1f) {
        str += "tmpPos = Reflect(tmpPos, vec3(0.0,1.0,0.0), " + node + ".transform[2].xyz, " + parentRot + ");\n";
	}
	if (nodeData.object[2][2] > 0.1f) {
		str += "tmpPos = Reflect(tmpPos, vec3(0.0,0.0,1.0), " + node + ".transform[2].xyz, " + parentRot + ");\n";
	}
	return str; 
}

std::string Scene::getAllShapesCode() {
	std::string shapesCode = "";
    for (auto& shapes : m_shapes) {
        for (auto& shape : shapes.second) {
			shapesCode += shape.code;
		}
	}
	return shapesCode + "\n";
}

std::string Scene::getShaderByName(std::string shaderName, Type type) {
    std::vector<ShaderShape> shapes = m_shapes[type];
    for (auto& shape : shapes) {
        if (shape.name == shaderName) {
            return shape.code;
        }
    }
    return "";
}

void Scene::setShaderByName(std::string name, std::string code, Type type) {
    for (auto& shape : m_shapes[type]) {
        if (shape.name == name) {
			shape.code = code;
			return;
		}
	}
    AddShape(name, code, type);
}

std::string Scene::getShaderCode() {
    m_shaderCode = getAllShapesCode();
    m_shaderCode += m_shaderBegin;
    m_shaderCode += "vec3 tmpPos = pos;\n";
    m_shaderCode += "mat3 rot;\n";
    if (m_sceneSize <= 1) {
		m_shaderCode += "return SDFData(vec4(1.0, 0.0, 0.0, 0.0), -1);}\n";
		return m_shaderCode;
	}
    std::vector<std::string> objectsShaders(m_sceneSize);
    for (int i = 0; i < m_sceneSize; i++) {
		NodeData node = m_nodeData[i];
        SceneGraphNode* sgNode = GetSceneGraphNode(node.data0.w);
        std::string nodeStr = "SceneNodes.nodes["+ std::to_string(i) +"]";
        bool hasMirror = (node.object[2][0] > 0.1f || node.object[2][1] > 0.1f || node.object[2][2] > 0.1f);
        if (node.data0.x > 0 && objectsShaders[node.data0.y] != "") { // not empty group
            std::string gName = "g" + std::to_string(i);
            std::string result = "";
            nodeStr = "SceneNodes.nodes[" + std::to_string(node.data0.y) + "]";
            hasMirror = (m_nodeData[node.data0.y].object[2][0] > 0.1f || m_nodeData[node.data0.y].object[2][1] > 0.1f || m_nodeData[node.data0.y].object[2][2] > 0.1f);
            result += "rot = Rotate(radians(" + nodeStr + ".transform[1].x), radians(" + nodeStr + ".transform[1].y), radians(" + nodeStr + ".transform[1].z));\n";
            if (hasMirror) {
				result += mirrirShader("SceneNodes.nodes[" + std::to_string(i) + "]", m_nodeData[node.data0.y]);
			}
            result += "SDFData "+gName + " = "+ objectsShaders[node.data0.y]+";\n";
            for (int j = 1; j < node.data0.x; ++j) {
                int childIndex = node.data0.y + j;
                nodeStr = "SceneNodes.nodes[" + std::to_string(childIndex) + "]";
                hasMirror = (m_nodeData[childIndex].object[2][0] > 0.1f || m_nodeData[childIndex].object[2][1] > 0.1f || m_nodeData[childIndex].object[2][2] > 0.1f);
                if (objectsShaders[childIndex] != "") {
                    result += "rot = Rotate(radians(" + nodeStr + ".transform[1].x), radians(" + nodeStr + ".transform[1].y), radians(" + nodeStr + ".transform[1].z));\n";
                    if (hasMirror) {
                        result += mirrirShader("SceneNodes.nodes[" + std::to_string(i) + "]", m_nodeData[childIndex]);
                    }
                    switch (m_nodeData[childIndex].data0.z) {
					    case Union:
						    result += gName + " = opU("+ objectsShaders[childIndex] +", "+gName+", "+ nodeStr +".data1.x, " + nodeStr + ".data1.y);\n";
						    break;
					    case Intersection:
						    result += gName + " = opI("+ objectsShaders[childIndex] +", "+gName+", " + nodeStr + ".data1.x, " + nodeStr + ".data1.y);\n";
						    break;
					    case Difference:
						    result += gName + " = opS("+ objectsShaders[childIndex] +", "+gName+", " + nodeStr + ".data1.x, " + nodeStr + ".data1.y);\n";
						    break;
				    }
                }
            }
            m_shaderCode += result;
            objectsShaders[i] = gName;
        }
        else if (node.data0.x == -1) { // object
            std::string p = (hasMirror) ? "tmpPos" : "pos";
            std::string pos = "(rot * ("+p+" - " + nodeStr + ".transform[2].xyz))";
            std::string shaderName = sgNode->getObject()->getComponent<Shape>()->getShaderName();
            if (node.object[1].w == 0) { // Sphere
                objectsShaders[i] = "SDFData(vec4("+ shaderName + "(" + pos + ", " + nodeStr + ".obejctData[0].x), " + nodeStr + ".color.xyz), " + std::to_string(node.data0.w) + ")";
            }
            else if (node.object[1].w == 1) { // Box
                objectsShaders[i] = "SDFData(vec4(" + shaderName + "("+pos+", " + nodeStr + ".obejctData[0].xyz, " + nodeStr + ".obejctData[0].w), " + nodeStr + ".color.xyz), " + std::to_string(node.data0.w) + ")";
            }
            else if (node.object[1].w == 2) { // Cone
                objectsShaders[i] = "SDFData(vec4(" + shaderName + "(" + pos + ", " + nodeStr + ".obejctData[0].x, " + nodeStr + ".obejctData[0].y, " + nodeStr + ".obejctData[0].z), " + nodeStr + ".color.xyz), " + std::to_string(node.data0.w) + ")";
            }
            else if (node.object[1].w == 3) { // Cylinder
            	objectsShaders[i] = "SDFData(vec4(" + shaderName + "(" + pos + ", " + nodeStr + ".obejctData[0].x, " + nodeStr + ".obejctData[0].y), " + nodeStr + ".color.xyz), " + std::to_string(node.data0.w) + ")";
            }
            else if (node.object[1].w == 4) { // Pyramid
            	objectsShaders[i] = "SDFData(vec4(" + shaderName + "(" + pos + ", " + nodeStr + ".obejctData[0].x, " + nodeStr + ".obejctData[0].y), " + nodeStr + ".color.xyz), " + std::to_string(node.data0.w) + ")";
            }
            else if (node.object[1].w == 5) { // Torus
            	objectsShaders[i] = "SDFData(vec4(" + shaderName + "(" + pos + ", " + nodeStr + ".obejctData[0].x, " + nodeStr + ".obejctData[0].y), " + nodeStr + ".color.xyz), " + std::to_string(node.data0.w) + ")";
            }
        }
	}
    if (objectsShaders[m_sceneSize - 1] == "") {
        m_shaderCode += "return SDFData(vec4(1.0, 0.0, 0.0, 0.0), -1);}\n";
        return m_shaderCode;
    }
    m_shaderCode += "return "+ objectsShaders[m_sceneSize-1] + ";\n}\n\n";
	return m_shaderCode;
}

void Scene::newScene() {
    if (!m_sceneGraph.isLeaf()) {
        for (auto& child : m_sceneGraph.getChildren()) {
            RemoveSceneGraphNode(child, false);
        }
    }
    LoadSceneDataFromResource(IDR_SYM_SCENE, m_sceneData);
    m_sceneSize = m_sceneData.sceneSize;
    for (int i = 0; i < m_sceneSize; i++) {
        m_nodeData[i] = m_sceneData.nodeData[i];
    }
    RecreateScene();
    m_tmpSceneData = m_sceneData;
    undoStack = UndoStack(m_maxUndoRedo);
    redoStack = UndoStack(m_maxUndoRedo);
    m_filename = "";
}