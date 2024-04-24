#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "../common/config.h"
#include "../engine/scene.h"


class Editor {
public:
	Editor();
	~Editor();
	void Docker();
	void SettingsPanel(Scene* scene);
	void MenuBar();
	glm::vec4 GetViewport();
	void setTheme();
	void Gizmo(Scene* scene);
private:
	ImGuiID dockspace_id;
	glm::vec4 m_maskScene;
	glm::vec4 m_maskCode;
	glm::vec4 m_maskInspector;
	float m_menuBarHeight;

	void calcViewport(glm::vec4 &viewportSize, glm::vec4 mask, float viewportX);
	void DrawSceneGraphNode(SceneGraphNode* node, bool draw, bool destroy, Scene* scene);

	void getScene(Scene* scene);
	void getInspector(Scene* scene);
};