#pragma once
#include "../common/config.h"
#include "object.h"
#include <vector>

enum BoolOperatios {
	Union,
	Intersection,
	Difference
};

class SceneGraphNode {
private:
	SceneGraphNode* m_parent;
	std::vector<SceneGraphNode*> m_children;
	bool m_hasObject;
	std::unique_ptr<GameObject> m_gameObject;
	Transform m_transform;
	BoolOperatios m_boolOperation = Union;
	float m_smoothingFactor;
	int m_id;
	std::string m_name;
public:
	SceneGraphNode(int id, bool hasObject = false, SceneGraphNode* parent = nullptr, std::string name = "");
	~SceneGraphNode();
	void addChild(SceneGraphNode* child);
	void removeChild(SceneGraphNode* child);
	void setParent(SceneGraphNode* parent);
	void updateObject();
	void addObject(std::unique_ptr<GameObject> object);
	void removeObject();
	void rename(std::string name);
	std::vector<SceneGraphNode*> getChildren() { return m_children; }
	bool isLeaf() { return m_children.size() == 0; }
	int getId() { return m_id; }
	std::string getName() { return m_name; }
	SceneGraphNode* getParent() { return m_parent; }
	Transform* getTransform() { return &m_transform; }
};