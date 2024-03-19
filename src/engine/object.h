#pragma once
#include <typeindex>
#include "../common/config.h"
#include "Component.h"
#include "Transform.h"
#include "Shape.h"

struct ObjectData {
    TransformStruct transform;
    glm::vec4 pos;
    //ShapeData shape;
    //ColorData color;
};

class GameObject {
    std::unordered_map<std::type_index, Component*> components;

public:
    
    GameObject() {
        Transform* transformComponent = new Transform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
        addComponent(transformComponent);
    }
    
    template<typename T>
    void addComponent(T* component) {
        components[typeid(T)] = component;
    }

    template<typename T>
    T* getComponent() {
        auto it = components.find(typeid(T));
        return it != components.end() ? static_cast<T*>(it->second) : nullptr;
    }

    ~GameObject() {
        for (auto& pair : components) {
            delete pair.second;
        }
    }
};
