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
	void MenuBar(Scene* scene);
	glm::vec4 GetViewport();
	void setTheme();
	void Gizmo(Scene* scene);
	void saveScene(Scene* scene, bool saveAs = false);
	void openScene(Scene* scene);
private:
	glm::vec4 xColor = glm::vec4(0.000f, 1.000f, 0.557f, 1.000f);
	glm::vec4 yColor = glm::vec4(1.000f, 0.000f, 0.502f, 1.000f);
	glm::vec4 zColor = glm::vec4(0.478f, 0.000f, 1.000f, 1.000f);
	int m_transformMode = 0;
	bool m_isMovingElement = false;
	ImGuiID dockspace_id;
	glm::vec4 m_maskScene;
	glm::vec4 m_maskCode;
	glm::vec4 m_maskInspector;
	float m_menuBarHeight;

	void calcViewport(glm::vec4 &viewportSize, glm::vec4 mask, float viewportX);
	void DrawSceneGraphNode(SceneGraphNode* node, bool draw, bool destroy, Scene* scene);

	void getScene(Scene* scene);
	void getInspector(Scene* scene);
	void getSettings(Scene* scene);
	bool hasCorrectExtension(const std::string& filename, const std::string& extension);
	bool drawFloat3(std::string label, glm::vec3& value, float speed = 0.01f, float min = 0.0f, float max = 0.0f);
	struct Keybinding {
		const char* key;
		const char* description;
	};

	// Define your keybindings
	const Keybinding keybindings[11] = {
		{"F", "Focus on selected object"},
		{"Delete", "Delete selected object"},
		{"Ctrl+C", "Copy selected object"},
		{"Ctrl+V", "Paste copied object"},
		{"Ctrl+D", "Duplicate selected object"},
		{"Ctrl+Z", "Undo last action"},
		{"Ctrl+Y", "Redo last undone action"},
		{"Ctrl+S", "Save current scene"},
		{"Ctrl+Shift+S", "Save current scene as"},
		{"Ctrl+N", "Create new scene"},
		{"Ctrl+O", "Open existing scene"}
	};
};