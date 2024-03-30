#include "scene.h"
#include <algorithm>

Scene::Scene()
{
    m_camera = Camera(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0,0.0,0.0), 0.0f, 2.5f);
    description = {};
    description.camera_position = m_camera.getPosition();
    description.camera_target = m_camera.getTarget();
    description.camera_roll = m_camera.getRoll();
    description.camera_fov = m_camera.getFov();
    description.sphereCount = 20;
    description.boxCount = 0;
    description.coneCount = 0;
    
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
        Transform* transformComponent = new Transform(glm::vec3((i% objCount)*0.1f -1.0f, 0.051f, ((int)i/ objCount)*0.1f), glm::vec3(0.0f), glm::vec3(0.05f));
        mygameObject->addComponent(transformComponent);
        
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
            // Note: We now use gameObjectPtr->get() to get the raw pointer from the unique_ptr
            data.transform = gameObjectPtr->getComponent<Transform>()->getStruct();
            data.pos = glm::vec4(gameObjectPtr->getComponent<Transform>()->getPosition(),0.0f);
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
