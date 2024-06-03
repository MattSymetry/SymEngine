#pragma once
#include "../common/config.h"
#include <stack>
#include <deque>
#include "cereal/types/vector.hpp"
#include "cereal/types/map.hpp"

struct SceneData {
    std::vector<NodeData> nodeData;
    std::vector<std::string> names;
    int sceneSize;
    int showGrid;
    int AA;
    int selectedId;
    glm::vec4 backgroundColor;
    glm::vec4 sunPosition;
    float outlineThickness;
    glm::vec4 outlineColor;
    std::map<Type, std::vector<ShaderShape>> shaderShapes;

    bool operator==(const SceneData& other) const {
        return sceneSize == other.sceneSize &&
            nodeData == other.nodeData &&
            showGrid == other.showGrid &&
            AA == other.AA &&
            selectedId == other.selectedId &&
            backgroundColor == other.backgroundColor &&
            sunPosition == other.sunPosition &&
            outlineThickness == other.outlineThickness &&
            outlineColor == other.outlineColor &&
            names == other.names;
            shaderShapes == other.shaderShapes;
    }

    template <class Archive>
    void serialize(Archive& archive) {
        archive(nodeData, names, sceneSize, showGrid, AA, selectedId, backgroundColor, sunPosition, outlineThickness, outlineColor, shaderShapes);
    }
};

class UndoStack {
private:
    std::deque<SceneData> stack;
    size_t maxSize;

public:
    UndoStack(size_t size) : maxSize(size) {}

    void push(const SceneData& sceneData) {
        if (stack.size() == maxSize) {
            stack.pop_front();
        }
        stack.push_back(sceneData);
    }

    void pop() {
        if (!stack.empty()) {
            stack.pop_back();
        }
    }

    SceneData top() const {
        return stack.back();
    }

    bool empty() const {
        return stack.empty();
    }

    void clear() {
        stack.clear();
    }

    size_t size() const {
        return stack.size();
    }
};