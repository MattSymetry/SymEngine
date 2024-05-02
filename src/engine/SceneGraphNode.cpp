#include "SceneGraphNode.h"

SceneGraphNode::SceneGraphNode(int id, bool hasObject, SceneGraphNode* parent, std::string name) : m_parent(parent) {
	m_id = id;
	m_dataId = id;
	m_name = name;
	m_hasObject = hasObject;
	if (m_parent) {
		m_parent->addChild(this);
		this->getTransform()->updateWorldSpace(parent->getTransform()->getTransform());
	} 
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

void SceneGraphNode::setParent(SceneGraphNode* parent) {
	if (m_parent) {
		m_parent->removeChild(this);
	}
	if (parent->getParent() == this) {
		parent->setParent(m_parent);
	}
	m_parent = parent;
	this->getTransform()->updateWorldSpace(parent->getTransform()->getTransform());
	m_parent->addChild(this);
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
	m_data.data1.x = operation;
}

NodeData* SceneGraphNode::getData() {
	m_data.data0.x = m_dataId;
	m_data.data1.z = m_id;
	m_data.data0.y = (m_parent != nullptr) ? m_parent->getDataId() : -1;
	//m_data.data0.z = -1;
	m_data.data0.w = m_children.size();
	m_data.data1.x = m_boolOperation;
	m_data.data1.y = m_isGroup;
	m_data.transform = m_transform.getWorldTransform();
	if (m_hasObject) {
		m_data.object = m_gameObject->getData();
	}
	else {
		m_data.object = {};
	}
	
	return &m_data;
}