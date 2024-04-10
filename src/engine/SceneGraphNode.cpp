#include "SceneGraphNode.h"

SceneGraphNode::SceneGraphNode(int id, bool hasObject, SceneGraphNode* parent, std::string name) : m_parent(parent) {
	m_id = id;
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