#include "SceneGraphNode.h"

SceneGraphNode::SceneGraphNode(int id, bool hasObject, SceneGraphNode* parent, std::string name) : m_parent(parent) {
	m_id = id;
	m_dataId = id;
	m_name = name;
	m_hasObject = hasObject;
	if (m_parent) {
		m_parent->addChild(this);
		this->getTransform()->updateWorldSpace();
	}
	m_color = glm::vec4(1.0f);
	m_goop = 0.1f;
}

void SceneGraphNode::addChild(SceneGraphNode* child) {
	m_children.push_back(child);
}

void SceneGraphNode::removeChild(SceneGraphNode* child) {
	auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end()) {
		m_children.erase(it);
	}
}

void SceneGraphNode::setParent(SceneGraphNode* parent, bool addToParent) {
	if (m_parent) {
		m_parent->removeChild(this);
	}
	if (parent->getParent() == this) {
		parent->setParent(m_parent);
	}
	m_parent = parent;
	this->getTransform()->updateWorldSpace(parent->getTransform()->getTransform());
	if (addToParent) m_parent->addChild(this);
}

void SceneGraphNode::addObject(std::unique_ptr<GameObject> object) {
	m_gameObject = std::move(object);
	m_hasObject = true;
}

void SceneGraphNode::removeObject() {
	m_gameObject->~GameObject();
	m_gameObject = nullptr;
	m_hasObject = false;
}

void SceneGraphNode::updateObject() {
	
}

void SceneGraphNode::rename(std::string name) {
	m_name = name;
}

SceneGraphNode::~SceneGraphNode() {
	if (m_parent) {
		m_parent->removeChild(this);
	}
}

void SceneGraphNode::changeBoolOperation(BoolOperatios operation) {
	m_boolOperation = operation;
	m_data.data0.z = operation;
}

void SceneGraphNode::addChildAfter(SceneGraphNode* child, SceneGraphNode* moveBehindNode) {
	child->getParent()->removeChild(child);
	child->setParent(this, false);

	auto it = std::find(m_children.begin(), m_children.end(), moveBehindNode);
	if (it != m_children.end()) {
		++it;
		m_children.insert(it, child);
	}
	else {
		m_children.push_back(child);
	}
}

NodeData* SceneGraphNode::getData() {
	m_data.data0.x = m_dataId;
	m_data.data0.y = (m_parent != nullptr) ? m_parent->getDataId() : -1; // TODO
	m_data.data0.z = m_parent ? m_parent->getBoolOperation() : -1;
	m_data.data0.w = m_id;
	m_data.data1.x = m_goop;
	m_data.color = m_color;
	m_data.transform = m_transform.getWorldTransform();
	if (m_hasObject) {
		m_data.object = m_gameObject->getData();
	}
	else {
		m_data.object = {};
	}
	
	return &m_data;
}

void SceneGraphNode::setData(const NodeData data) {
	m_data = data;
	m_boolOperation = static_cast<BoolOperatios>(data.data0.z);
	m_goop = data.data1.x;
	m_color = data.color;
	m_hasObject = data.data0.x <= 0;// TODO group in group
	m_isGroup = !m_hasObject;
	if (m_hasObject) {
		GameObject* obj = new GameObject();
		Shape* shape = new Shape(Shape::createShape(static_cast<Type>(data.object[3][3])));
		ShapeDataStruct shapeData;
		shapeData.parameters = data.object;
		shapeData.type = static_cast<int>(data.object[3][3]);
		shape->setShapeData(shapeData);
		obj->addComponent(shape);
		addObject(std::unique_ptr<GameObject>(obj));
	}
	m_transform.setWorldPosition(data.transform[2]);
	m_transform.setWorldRotation(data.transform[1]);
}