#pragma once
#include "../common/config.h"
#include "object.h"
#include <vector>

namespace glm {
	template<class Archive> void serialize(Archive& archive, glm::vec4& v) {
		archive(v.x, v.y, v.z, v.w);
	}
	template<class Archive> void serialize(Archive& archive, glm::ivec4& v) {
		archive(v.x, v.y, v.z, v.w);
	}
	template<class Archive> void serialize(Archive& archive, glm::mat4& m) {
		archive(m[0], m[1], m[2], m[3]);
	}
}

enum BoolOperatios {
	Union = 0,
	Intersection = 1,
	Difference = 2
};

struct NodeData {
	glm::ivec4 data0;//childCount, childStart, operation, sceneID
	glm::vec4 data1;// operatorGoop,colorGoop, padding[2]
	glm::mat4 transform; 
	glm::mat4 object;
	glm::vec4 color;

	bool operator==(const NodeData& other) const {
		return data0 == other.data0 &&
			data1 == other.data1 &&
			transform == other.transform &&
			object == other.object &&
			color == other.color;
	}

	template <class Archive>
	void serialize(Archive& archive) {
		archive(data0, data1, transform, object, color);
	}
};

class SceneGraphNode {
private:
	NodeData m_data;
	SceneGraphNode* m_parent;
	std::vector<SceneGraphNode*> m_children;
	bool m_hasObject;
	std::unique_ptr<GameObject> m_gameObject;
	Transform m_transform;
	bool m_isGroup = true;
	BoolOperatios m_boolOperation = Union;
	float m_smoothingFactor;
	int m_id;
	int m_dataId = -1;
	std::string m_name;
	glm::vec4 m_color;
	float m_goop;
	float m_colorGoop;
	void copyFrom(const SceneGraphNode& other) {
		m_data = other.m_data;
		m_parent = other.m_parent;
		//m_children = other.m_children;
		m_hasObject = other.m_hasObject;
		if (other.m_gameObject) {
			GameObject* gObj = new GameObject();
			Shape* shape = new Shape(Shape::createShape(other.m_gameObject->getComponent<Shape>()->getType()));
			shape->setShapeData(other.m_gameObject->getComponent<Shape>()->getStruct());
			gObj->addComponent(shape);
			m_gameObject = std::unique_ptr<GameObject>(gObj);
			m_MirrorX = other.m_MirrorX;
			m_MirrorY = other.m_MirrorY;
			m_MirrorZ = other.m_MirrorZ;
		}
		else {
			m_gameObject.reset();
		}
		m_transform = other.m_transform;
		m_isGroup = other.m_isGroup;
		m_boolOperation = other.m_boolOperation;
		m_smoothingFactor = other.m_smoothingFactor;
		m_id = 9999;
		m_dataId = other.m_dataId;
		m_name = other.m_name;
		m_color = other.m_color;
		m_goop = other.m_goop;
		m_colorGoop = other.m_colorGoop;
		// children
		for (SceneGraphNode* child : other.m_children) {
			SceneGraphNode* newChild = new SceneGraphNode();
			newChild->copyFrom(*child);
			m_children.push_back(newChild);
		}
	}
public:
	SceneGraphNode(int id, bool hasObject = false, SceneGraphNode* parent = nullptr, std::string name = "");
	SceneGraphNode() : m_id(0), m_hasObject(false), m_parent(nullptr), m_name(""), m_isGroup(true), m_boolOperation(Union), m_smoothingFactor(0.0f), m_dataId(-1), m_goop(0.1f) {
		// Initialize other members if necessary
	}
	~SceneGraphNode();
	SceneGraphNode(const SceneGraphNode& other) {
		copyFrom(other);
	}
	SceneGraphNode& operator=(const SceneGraphNode& other) {
		if (this != &other) {
			copyFrom(other);
		}
		return *this;
	}
	void addChild(SceneGraphNode* child);
	void removeChild(SceneGraphNode* child);
	void setParent(SceneGraphNode* parent, bool addToParent = true);
	void updateObject();
	void addObject(std::unique_ptr<GameObject> object);
	void removeObject();
	void rename(std::string name);
	void changeBoolOperation(BoolOperatios operation);
	std::vector<SceneGraphNode*> getChildren() { return m_children; }
	int getChildCount() { return m_children.size(); }
	bool isLeaf() { return m_children.size() == 0; }
	int getId() { return m_id; }
	GameObject* getObject() { return m_gameObject.get(); }
	std::string getName() { return m_name; }
	SceneGraphNode* getParent() { return m_parent; }
	Transform* getTransform() { return &m_transform; }
	NodeData* getData();
	void setIsGroup(bool isGroup) { m_isGroup = isGroup; }
	bool isGroup() { return m_isGroup; }
	BoolOperatios getBoolOperation() { return m_boolOperation; }
	int getDataId() { return m_dataId; }
	void setDataId(int dataId) { m_dataId = dataId; m_data.data0.x = dataId; }
	void setColor(glm::vec4 color) { m_color = color; m_data.color = color; }
	glm::vec4 getColor() { return m_color; }
	void addChildAfter(SceneGraphNode* child, SceneGraphNode* moveBehindNode);
	void setGoop(float goop) { m_goop = goop; m_data.data1.x = goop; }
	float getGoop() { return m_goop; }
	void setColorGoop(float colorGoop) { m_colorGoop = colorGoop; m_data.data1.y = colorGoop; }
	float getColorGoop() { return m_colorGoop; }
	void setData(const NodeData data);
	void setId(int id) { m_id = id; m_data.data0.w = id; }
	bool m_MirrorX = false;
	bool m_MirrorY = false;
	bool m_MirrorZ = false;
	void setMirrorX(bool mirror) { m_MirrorX = mirror; m_data.object[2][0] = mirror ? 1.0f : 0.0f; }
	void setMirrorY(bool mirror) { m_MirrorY = mirror; m_data.object[2][1] = mirror ? 1.0f : 0.0f; }
	void setMirrorZ(bool mirror) { m_MirrorZ = mirror; m_data.object[2][2] = mirror ? 1.0f : 0.0f; }
	bool getMirrorX() { return m_MirrorX; }
	bool getMirrorY() { return m_MirrorY; }
	bool getMirrorZ() { return m_MirrorZ; }
};