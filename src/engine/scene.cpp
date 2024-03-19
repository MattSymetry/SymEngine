#include "scene.h"

Scene::Scene()
{
    m_camera = Camera(glm::vec3(0.0f, 453.0f, 0.0f), glm::vec3(0.0,0.0,0.0), glm::vec3(0.0f, 0.0f, 0.0f), 2.5f);
    description = {};
    description.camera_position = m_camera.getPosition();
    description.camera_forward = m_camera.getForward();
    description.camera_up = m_camera.getUp();
    description.camera_right = m_camera.getRight();
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
        Transform* transformComponent = new Transform(glm::vec3((i%20)*0.1f -1.0f, 0.25f, ((int)i/20)*0.1f), glm::vec3(0.0f), glm::vec3(0.05f));
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
    float cameraSpeed = 0.5f * m_deltaTime / 1000;
    if (state[SDL_SCANCODE_W]) {
        m_camera.MoveForward(cameraSpeed);
	}
    if (state[SDL_SCANCODE_S]) {
        m_camera.MoveBackward(cameraSpeed);
	}
    if (state[SDL_SCANCODE_A]) {
        m_camera.MoveLeft(cameraSpeed);
	}
    if (state[SDL_SCANCODE_D]) {
        m_camera.MoveRight(cameraSpeed);
    }
    if (state[SDL_SCANCODE_SPACE]) {
        m_camera.MoveUp(cameraSpeed);
	}
    if (state[SDL_SCANCODE_LSHIFT]) {
        m_camera.MoveDown(cameraSpeed);
	}
    if (state[SDL_SCANCODE_LEFT]) {
        m_camera.RotateLeft(cameraSpeed);
	}
    if (state[SDL_SCANCODE_RIGHT]) {
        m_camera.RotateRight(cameraSpeed);
	}
    if (state[SDL_SCANCODE_UP]) {
        m_camera.RotateUp(cameraSpeed);
	}
    if (state[SDL_SCANCODE_DOWN]) {
        m_camera.RotateDown(cameraSpeed);
    }
}

void Scene::Update(int deltaTime) {
    m_deltaTime = deltaTime;
    frameCount ++;
    //std::cout << "forward: " << frameCount << std::endl;

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
