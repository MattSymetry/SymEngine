#pragma once
#include <typeindex>
#include "../common/config.h"
#include "Component.h"
#include "Transform.h"
#include "Shape.h"

struct ObjectData {
    TransformStruct transform;
    ShapeDataStruct shape; 
    //ColorData color;
};

class GameObject {
private:
    ObjectData m_data;
    std::unordered_map<std::type_index, Component*> m_components;
    void updateData() {
		Transform* transform = getComponent<Transform>();
		if (transform) {
			m_data.transform = transform->getStruct();
		}
		Shape* shape = getComponent<Shape>();
		if (shape) {
			m_data.shape = shape->getStruct();
		}
	}

public:
    
    GameObject() {
        Transform* transformComponent = new Transform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
        addComponent(transformComponent);
    }
    
    template<typename T>
    void addComponent(T* component) {
        m_components[typeid(T)] = component;
    }

    template<typename T>
    T* getComponent() {
        auto it = m_components.find(typeid(T));
        return it != m_components.end() ? static_cast<T*>(it->second) : nullptr;
    }

    ~GameObject() {
        for (auto& pair : m_components) {
            delete pair.second;
        }
    }
};
