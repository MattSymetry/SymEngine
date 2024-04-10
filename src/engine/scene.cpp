#include "scene.h"
#include <algorithm>

Scene::Scene(glm::vec4 viewport) : m_sceneGraph(0, false, nullptr, "Scene")
{
    m_sceneGraphNodes.push_back(&m_sceneGraph);
    m_camera = Camera(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0,0.0,0.0), 0.0f, 2.5f);
    description = {};
    description.camera_position = m_camera.getPosition();
    description.camera_target = m_camera.getTarget();
    description.viewport = viewport;
    description.camera_roll = m_camera.getRoll();
    description.camera_fov = m_camera.getFov();
    description.sphereCount = 20;
    description.boxCount = 0;
    description.coneCount = 0;

    m_viewport = viewport;
    
    AddBuffer(sizeof(description), vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &description);
    AddBuffer(4, vk::BufferUsageFlagBits::eUniformBuffer, vk::DescriptorType::eUniformBuffer, &frameCount);
    SetupObjects();
    
    std::cout << gameObjects.size();
}

void Scene::AddBuffer(size_t size, vk::BufferUsageFlagBits usage, vk::DescriptorType descriptorType, void *dataPtr) {
    buffers.push_back({
        size, usage, descriptorType, dataPtr
    });
}

void Scene::SetupObjects() {
    int objCount = description.sphereCount;
    for (int i = 0; i < objCount; i++) {
        auto mygameObject = std::make_unique<GameObject>();
        mygameObject->getComponent<Transform>()->setPosition(glm::vec3((i% objCount)*0.1f -1.0f, 0.051f, ((int)i/ objCount)*0.1f));
        mygameObject->getComponent<Transform>()->setScale(glm::vec3(0.05f));
        Sphere sphereShape;
        sphereShape.radius = 5.0f;
        mygameObject->addComponent(new Shape(sphereShape));
        
        gameObjects.push_back(std::move(mygameObject));
    };
    
    objectData.reserve(objCount);
    float i = 1;
    for (const auto& gameObjectPtr : gameObjects) {
        if (gameObjectPtr) {
            ObjectData data;
            data.transform = gameObjectPtr->getComponent<Transform>()->getStruct();
            std::cout << data.transform.position.x << " " << data.transform.position.y << " " << data.transform.position.z << std::endl;
            objectData.push_back(data);
            i++;
        }
    }
    AddBuffer(sizeof(ObjectData)* objCount, vk::BufferUsageFlagBits::eStorageBuffer, vk::DescriptorType::eStorageBuffer, objectData.data());
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
    

    //description.camera_position = _camera.getPosition();
    //std::count << description.camera_position.x << " " << description.camera_position.y << " " << description.camera_position.z << std::endl;
    //description.camera_forward = _camera.getForward();
    //description.camera_up = _camera.getUp();
    //gameObjects[0]->getComponent<Transform>()->setPosition(_camera.getPosition() + _camera.getForward()*8.0f);
    //_camera.LookAt(gameObjects[0]->getComponent<Transform>()->getPosition());
    //_camera.Update();
    //UpdateObjectData();
}

void Scene::UpdateObjectData() {
	int i = 0;
    for (const auto& gameObjectPtr : gameObjects) {
        if (gameObjectPtr) {
            ObjectData* data = &objectData[i];
            data->transform = gameObjectPtr->getComponent<Transform>()->getStruct();
            i++;
        }
    }

}

void Scene::UpdateViewport(glm::vec4 viewport) {
	m_viewport = viewport;
	description.viewport = viewport;
}

void Scene::AddEmpty(SceneGraphNode* parent) {
    SceneGraphNode* node = AddSceneGraphNode();
    if (parent) {
		node->setParent(parent);
	}
}

void Scene::AddSphere(glm::vec3 position, float radius, glm::vec3 color) {
	auto mygameObject = std::make_unique<GameObject>();
	mygameObject->getComponent<Transform>()->setPosition(position);
	Sphere sphereShape;
	sphereShape.radius = radius;
	mygameObject->addComponent(new Shape(sphereShape));
	
    SceneGraphNode* node = AddSceneGraphNode();
    node->addObject(std::move(mygameObject));
}

SceneGraphNode* Scene::AddSceneGraphNode() {
    m_sceneSize++;
    SceneGraphNode* node = new SceneGraphNode(m_sceneSize, false, &m_sceneGraph, "Object " + std::to_string(m_sceneSize));
    m_sceneGraphNodes.push_back(node);
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
    if (node) {
        if (!node->isLeaf()) {
            for (auto& child : node->getChildren()) {
				RemoveSceneGraphNode(child);
			}
		}
        m_sceneGraphNodes[node->getId()] = nullptr;
        delete node;
	}
}