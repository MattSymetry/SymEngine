#include "scene.h"
#include <algorithm>

Scene::Scene(glm::vec4 viewport) : m_sceneGraph(0, false, nullptr, "Scene")
{
    m_sceneSize = 1; 
    m_sceneGraphNodes.push_back(&m_sceneGraph);
    float aspect = viewport.z / viewport.w;
    m_camera = Camera(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0,0.0,0.0), 0.0f, 90.0f, aspect);
    description = {}; 
    description.camera_position = m_camera.getPosition();
    description.camera_target = m_camera.getTarget();
    description.viewport = viewport; 
    description.camera_roll = m_camera.getRoll(); 
    description.camera_fov = m_camera.getFov();
     description.sceneSize = m_sceneSize;

    m_viewport = viewport; 
    AddBuffer(sizeof(description), vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &description);
    AddBuffer(4, vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &frameCount);
    SetupObjects();
    updateNodeData();
    AddBuffer(sizeof(NodeData) * m_maxObjects, vk::BufferUsageFlagBits::eStorageBuffer, vk::DescriptorType::eStorageBuffer, m_nodeData.data());
}

void Scene::AddBuffer(size_t size, vk::BufferUsageFlagBits usage, vk::DescriptorType descriptorType, void *dataPtr) {
    buffers.push_back({
        size, usage, descriptorType, dataPtr
    });
}

void Scene::SetupObjects() {
    
}

void Scene::KeyPressed(SDL_Keycode key) {
    
}
 
void Scene::KeyboardInput(Uint8* state) {

}
 
void Scene::MouseInput(int x, int y) {
    m_camera.Orbit(glm::vec2(x, y), m_deltaTime / 1000.0);
    description.camera_position = m_camera.getPosition();
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
        SceneGraphNode* node = GetSceneGraphNode(data->data1.z);
        if (node) {
            //node->setDataId(i);
            //data->data0.x = i;
            //data->data0.y = node->getParent() ? node->getParent()->getDataId() : -1;
            //data->data0.z = i + 1;
            //data->data0.w = node->getChildren().size();
            data->transform = node->getTransform()->getWorldTransform();
           
            if (!node->isGroup()) {
                data->object = node->getObject()->getData();
            }
            else
            {
                data->data1.x = node->getBoolOperation();
            }
            //std::cout << "Node " << i << " " << node->getName() << std::endl;
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
    SerializeNode(&m_sceneGraph, -1, 0);
	description.sceneSize = m_sceneSize;
}

void Scene::SerializeNode(SceneGraphNode* node, int parentIndex, int index) {
    if (node == nullptr) {
        return; 
    }
    m_tmpNodeIndex++;
    if (index == 0) m_tmpNodeIndex = 0;
    NodeData* data = node->getData(); 
    node->setDataId(index);
    data->data0.y = parentIndex;
    data->data0.z = index + 1;
    auto children = node->getChildren();
    data->data0.w = children.size();

    data->data1.x = node->getBoolOperation();
    data->data1.y = node->isGroup();
    if (!data->data1.y) {
        data->object = node->getObject()->getData();
    }
    data->transform = node->getTransform()->getWorldTransform();

    //m_nodeData.push_back(*data); s
    m_nodeData[index] = *data;
    for (size_t i = 0; i < children.size(); ++i) {
        SerializeNode(children[i], index, m_tmpNodeIndex+1);
    }

    if (children.empty()) {
        m_nodeData[index].data0.z = -1;
    }
}

void Scene::AddEmpty(SceneGraphNode* parent, bool isObject) {
    SceneGraphNode* node = AddSceneGraphNode((isObject ? "Object" : "Group"));
    if (parent) {
		node->setParent(parent);
	}
	if (isObject) {
		node->setIsGroup(false);
        GameObject* obj = new GameObject();
        Sphere sphereShape;
        obj->addComponent(new Shape(sphereShape));
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