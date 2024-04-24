#pragma once
#include <typeindex>
#include "../common/config.h"
#include "Component.h"
#include "Transform.h"
#include "Shape.h"

struct ObjectData {
    int type;
    ShapeDataStruct shape;
    //ColorData color;
};

class GameObject {
private:
    glm::mat4 m_data;
    std::unordered_map<std::type_index, Component*> m_components;
    void updateData() {
		Shape* shape = getComponent<Shape>();
		if (shape) {
			m_data = shape->getParams();
		}
	}

public:
    
    GameObject() {

    }
    
    template<typename T>
    void addComponent(T* component) {
        m_components[typeid(T)] = component;
    }

    template<typename T>
    T* getComponent() {
        if (m_components.empty()) return nullptr;
        auto it = m_components.find(typeid(T));
        return it != m_components.end() ? static_cast<T*>(it->second) : nullptr;
    }

    glm::mat4 getData() {
		updateData();
		return m_data;
	}

    ~GameObject() {
        for (auto& pair : m_components) {
            delete pair.second;
        }
    }
};
