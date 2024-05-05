#pragma once
#include "../common/config.h"
#include "object.h"
#include <vector>

enum BoolOperatios {
	Union = 0,
	Intersection = 1,
	Difference = 2
};

struct NodeData {
	glm::ivec4 data0;//childCount, childStart, operation, sceneID
	//glm::ivec4 data1;// operator, group, sceneID, padding[1]
	glm::mat4 transform; 
	glm::mat4 object;
	glm::vec4 color;
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
};