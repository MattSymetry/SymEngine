#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "../common/config.h"
#include "../engine/scene.h"
#include "TextEditor.h"
#include "../Vulkan/vkUtil/shaders.h"

struct GlslError {
	int line;
	std::string message;
};


class Editor {
public:
	Editor(Scene* scene);
	~Editor();
	void Docker();
	void SettingsPanel(Scene* scene);
	void MenuBar(Scene* scene);
	glm::vec4 GetViewport();
	void setTheme();
	void Gizmo(Scene* scene);
	void saveScene(Scene* scene, bool saveAs = false);
	void openScene(Scene* scene);
	void openAddPanel(ImGuiContext* context, Scene* scene);
	void AddPanel(Scene* scene);
	bool codeEditorIsActive();
	bool getRenderImage() {return m_renderImage;}
	void setRenderImage(bool renderImage) {m_renderImage = renderImage;}
	glm::vec2 getImageSize() {return m_imageSize;}
private:
	glm::vec4 xColor = glm::vec4(0.000f, 1.000f, 0.557f, 1.000f);
	glm::vec4 yColor = glm::vec4(1.000f, 0.000f, 0.502f, 1.000f);
	glm::vec4 zColor = glm::vec4(0.478f, 0.000f, 1.000f, 1.000f);
	int m_transformMode = 0;
	bool m_isMovingElement = false;
	bool m_renderImage = false;
	glm::ivec2 m_imageSize = glm::ivec2(2048, 2048);
	std::string m_defaultCode = "Please select an object to see and edit its code!";
	int m_selectedCode = 0;
	int m_shaderCodeLinesOffset = 0;
	TextEditor m_editor;
	ImGuiID dockspace_id;
	glm::vec4 m_maskScene;
	glm::vec4 m_maskCode;
	glm::vec4 m_maskInspector;
	float m_menuBarHeight;
	bool m_autoCompile = false;
	std::string m_newFunctionName = "";

	void calcViewport(glm::vec4 &viewportSize, glm::vec4 mask, float viewportX);
	void DrawSceneGraphNode(SceneGraphNode* node, bool draw, bool destroy, Scene* scene);

	void getScene(Scene* scene);
	void getInspector(Scene* scene);
	void getSettings(Scene* scene);
	void getCode(Scene* scene);
	bool hasCorrectExtension(const std::string& filename, const std::string& extension);
	bool drawFloat3(std::string label, glm::vec3& value, float speed = 0.01f, float min = 0.0f, float max = 0.0f);
	struct Keybinding {
		const char* key;
		const char* description;
	};
	void setCodeLineOffset();

	std::vector<GlslError> m_errors;

	std::vector<GlslError> parseGlslErrors(const std::string& errorLog) {
		std::vector<GlslError> errors;
		std::regex errorRegex(R"(ERROR:\s+0:(\d+):\s+(.*))");
		std::smatch match;
		std::string::const_iterator searchStart(errorLog.cbegin());

		while (std::regex_search(searchStart, errorLog.cend(), match, errorRegex)) {
			int line = std::stoi(match[1].str());
			std::string message = match[2].str();
			errors.push_back({ line, message });
			searchStart = match.suffix().first;
		}

		return errors;
	}


	std::string getSelectedCode(Scene* scene) {

	}

	// Define your keybindings
	const Keybinding keybindings[12] = {
		{"F", "Focus on selected element"},
		{"Delete", "Delete selected element"},
		{"Ctrl+C", "Copy selected element"},
		{"Ctrl+V", "Paste copied element"},
		{"Ctrl+D", "Duplicate selected element"},
		{"Ctrl+Z", "Undo last action"},
		{"Ctrl+Y", "Redo last undone action"},
		{"Ctrl+S", "Save current scene"},
		{"Ctrl+Shift+S", "Save current scene as"},
		{"Ctrl+N", "Create new scene"},
		{"Ctrl+O", "Open existing scene"},
		{"Shift+A", "Open popup to add element"}
	};
};