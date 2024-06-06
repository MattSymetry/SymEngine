#pragma once
#include "Editor.h"
#include "glm/gtc/type_ptr.hpp"
#include "tinyfiledialogs.h"

Editor::Editor(Scene* scene)
{
    m_maskScene = glm::vec4(0.0f);
    m_maskCode = glm::vec4(0.0f);
    m_maskInspector = glm::vec4(0.0f);
    auto lang = TextEditor::LanguageDefinition::GLSL();
    m_editor.SetLanguageDefinition(lang);
    m_editor.SetPalette(TextEditor::GetDarkPalette());
    m_editor.SetText(m_defaultCode);
    std::set<int> readOnlyLines = { 0, 1 };
    m_editor.SetReadOnlyLines(readOnlyLines);
    setCodeLineOffset();

}

Editor::~Editor()
{
}

bool Editor::codeEditorIsActive()
{
    return m_editor.IsFocused();
}

void Editor::setCodeLineOffset()
{
    std::vector<char> shader = vkUtil::prepareShader();
    std::string str(shader.begin(), shader.end());
    std::string::size_type pos = 0;
    while ((pos = str.find('\n', pos)) != std::string::npos) {
        ++pos;
        ++m_shaderCodeLinesOffset;
    }
    std::cout << "Shader code lines offset: " << m_shaderCodeLinesOffset << std::endl;
}

bool Editor::hasCorrectExtension(const std::string& filename, const std::string& extension) {
    return filename.size() >= extension.size() &&
        filename.compare(filename.size() - extension.size(), extension.size(), extension) == 0;
}

bool Editor::drawFloat3(std::string label, glm::vec3& value, float speed, float min, float max) {
	bool changed = false;
    float width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f ;
    ImGui::PushItemWidth(width);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(xColor.x, xColor.y, xColor.z, xColor.w));
    if (ImGui::DragFloat(("##X"+label).c_str(), &value.x, speed, min, max, "%.3f")) {
        changed = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    // spacer on x of 
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(yColor.x, yColor.y, yColor.z, yColor.w));
    if (ImGui::DragFloat(("##Y" + label).c_str(), &value.y, speed, min, max, "%.3f")) {
		changed = true;
	}
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(zColor.x, zColor.y, zColor.z, zColor.w));
    if (ImGui::DragFloat(("##Z" + label).c_str(), &value.z, speed, min, max, "%.3f")) {
        changed = true;
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    return changed;
}

glm::vec4 Editor::GetViewport()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
    glm::vec4 viewportSize = glm::vec4(0.0f, 0.0f, viewport->Size.x, viewport->Size.y);
    if (m_maskScene != glm::vec4(0.0f))
	{
        calcViewport(viewportSize, m_maskScene, viewport->Size.x);
	}
	if (m_maskCode != glm::vec4(0.0f))
	{
        calcViewport(viewportSize, m_maskCode, viewport->Size.x);
	}
	if (m_maskInspector != glm::vec4(0.0f))
	{
        calcViewport(viewportSize, m_maskInspector, viewport->Size.x);
	}
	return viewportSize;
}

void Editor::calcViewport(glm::vec4& viewportSize, glm::vec4 mask, float viewportX)
{

    if (mask.y - m_menuBarHeight <= 0.0f) // up
    {
        if (mask.x <= 0.0f) // left
        {
            viewportSize.x = mask.z;
            viewportSize.z -= mask.z;
        }
        else if ((mask.x + mask.z) >= viewportX) // right
        {
            viewportSize.z -= mask.z;
        }
        else {
            viewportSize.w -= mask.y + mask.w;
        }
    }
    else // bottom
    {
        viewportSize.y += mask.w;
        viewportSize.w -= mask.w;
    }
}

void Editor::Docker()
{
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        static auto first_time = true;
        if (first_time)
        {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.05f, nullptr, &dockspace_id);

            ImGui::DockBuilderDockWindow(ICON_LC_CODE_XML " Code", dock_id_down);
            ImGui::DockBuilderDockWindow(ICON_LC_LAYERS_3 " Scene", dock_id_left);
            ImGui::DockBuilderDockWindow(ICON_LC_SETTINGS " Scene Settings", dock_id_left);
            ImGui::DockBuilderDockWindow(ICON_LC_TEXT_CURSOR_INPUT" Inspector", dock_id_right);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }
}


void Editor::getScene(Scene* scene)
{
    if (scene != nullptr)
    {
        float avail = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(avail);
        if (ImGui::Button(ICON_LC_BOXES " Add Group", ImVec2(avail, 0))) {
            scene->AddEmpty(scene->GetSelectedNode(), false);
		}
        float buttonWidth = avail * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f;
        int i = 0;
        for (const auto& object : NameToTypeMap)
        {
            i++;
            if (i % 2 == 0 && i != 0) { ImGui::SameLine(); }
            if (ImGui::Button(object.first.c_str(), ImVec2(buttonWidth, 0))) { 
                scene->AddEmpty(scene->GetSelectedNode(), true, object.second);
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::BeginDisabled();
        std::string objectsCounter = std::to_string(scene->getSceneSize()-1) + " / " + std::to_string(scene->m_maxObjects-1) + std::string(" objects");
        ImGui::Button(objectsCounter.c_str(), ImVec2(avail,0));
        ImGui::EndDisabled();

        ImGui::Spacing();

        SceneGraphNode* sceneGraph = scene->GetSceneGraph();
        DrawSceneGraphNode(sceneGraph, true, false, scene);
        if (ImGui::IsMouseReleased(0))
        {
            m_isMovingElement = false;  
        }

        bool createGroup = false;
        bool createObject = false;
        Type objectType = Type::Sphere;
        if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(1)) {
            ImGui::OpenPopup("Scene Popup");
        }
        if (ImGui::BeginPopup("Scene Popup"))
        {
            if (ImGui::Selectable(ICON_LC_BOXES " Add Group"))
            {
                createGroup = true;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::BeginMenu(ICON_LC_PLUS " Add Object"))
            {
                // List of all object types you can add
                for (const auto& object : NameToTypeMap)
                {
                    if (ImGui::Selectable(object.first.c_str()))
                    {
                        createObject = true;
                        objectType = NameToTypeMap.find(object.first)->second;
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        if (createGroup)
        {
            scene->AddEmpty(sceneGraph, false);
        }
        else if (createObject)
        {
            scene->AddEmpty(sceneGraph, true, objectType);
        }
    }
}

void Editor::DrawSceneGraphNode(SceneGraphNode* node, bool draw, bool destroy, Scene* scene)
{
	if (node != nullptr)
	{

        bool nodeOpen = draw;
        bool destroyEntity = destroy;
        bool createGroup = false;
        bool createObject = false;
        bool isGroup = node->isGroup();
        int id = node->getId();
        int selectedId = scene->GetSelectedId();
        Type objectType = Type::Sphere;

        const bool leaf = node->isLeaf();

        if (draw)
        {
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
            if (!leaf) { nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow; }
            else { nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; }

            if (id == selectedId) { nodeFlags |= ImGuiTreeNodeFlags_Selected; }

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            nodeFlags |= ImGuiTreeNodeFlags_SpanAvailWidth;
            std::string suff = (id == 0 && scene->hasSceneChanged()) ? " *" : "";
            std::string pre = "";
            if (id != 0) {
                if (node->getBoolOperation() == BoolOperatios::Union) { pre = ICON_LC_SQUARE_PLUS " "; }
			    else if (node->getBoolOperation() == BoolOperatios::Intersection) { pre = ICON_LC_SQUARE_SLASH " "; }
			    else if (node->getBoolOperation() == BoolOperatios::Difference) { pre = ICON_LC_SQUARE_MINUS " "; }

                if (isGroup) { pre += ICON_LC_BOXES " "; }
				else { 
                    switch (node->getObject()->getComponent<Shape>()->getType())
                    {
                        case Type::Sphere:
						    pre += ICON_LC_CIRCLE " ";
						break;
						case Type::Box:
                            pre += ICON_LC_CUBOID " ";
                            break;
                        case Type::Cone:
                            pre += ICON_LC_CONE " ";
							break;
                        case Type::Cylinder:
							pre += ICON_LC_CYLINDER " ";
							break;
                        case Type::Pyramid:
							pre += ICON_LC_PYRAMID " ";
							break;
                        case Type::Torus:
                            pre += ICON_LC_TORUS " ";
                            break;
                    default:
                        break;
                    }
                }
            }
            nodeOpen =
                ImGui::TreeNodeEx((void*)(intptr_t)id, nodeFlags, "%s", (pre+node->getName()+suff).c_str());
            ImGui::PopItemWidth();
            if (ImGui::IsItemClicked()) {
                scene->SetSelectedId(id);
                selectedId = id;
            } 

            const std::string entityPopupName = "Entity Popup " + std::to_string(id);
            if (ImGui::IsItemClicked(1))
            {
                ImGui::OpenPopup(entityPopupName.c_str());
            }

            if (ImGui::BeginPopup(entityPopupName.c_str()))
            {
                if (isGroup)
                {
                    if (ImGui::Selectable(ICON_LC_BOXES " Add Group"))
                    {
                        createGroup = true;
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::BeginMenu(ICON_LC_PLUS " Add Object"))
                    {
                        // List of all object types you can add
                        for (const auto& object : NameToTypeMap)
                        {
                            if (ImGui::Selectable(object.first.c_str()))
                            {
                                createObject = true;
                                objectType = NameToTypeMap.find(object.first)->second;
                                ImGui::CloseCurrentPopup();
                            }
                        }
                        ImGui::EndMenu();
                    }
                }
                if (id != 0)
                {
                    if (ImGui::Selectable(ICON_LC_TRASH_2 " Delete Object"))
                    {
                        destroyEntity = true;
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }

            if (id != 0)
            {
                ImGuiDragDropFlags srcFlags = 0;
                srcFlags |= ImGuiDragDropFlags_SourceNoDisableHover;
                srcFlags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
                if (ImGui::BeginDragDropSource(srcFlags))
                {
                    if (!(srcFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                    {
                        ImGui::Text("Moving Entity \"%s\"", node->getName().c_str());
                        m_isMovingElement = true;
                    }
                    ImGui::SetDragDropPayload("DND_DEMO_NAME", &node, sizeof(SceneGraphNode*));
                    ImGui::EndDragDropSource();
                }
            }

            if (ImGui::BeginDragDropTarget() && isGroup)
            {
                ImGuiDragDropFlags targetFlags = 0;
                if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("DND_DEMO_NAME", targetFlags))
                {
                    SceneGraphNode* moveFrom = *(SceneGraphNode**) payload->Data;
                    SceneGraphNode* moveTo = node;
                    moveFrom->setParent(moveTo);
                    scene->updateNodeData();
                } 
                ImGui::EndDragDropTarget();
            }
            
            if (m_isMovingElement && selectedId != id && !isGroup)
            {
                ImVec4 buttonColor = ImVec4(0.5f, 0.5f, 0.5f, 0.4f);
                ImVec4 hoverColor = ImVec4(0.8f, 0.8f, 0.8f, 0.6f);

                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
                ImGui::Button("###after"+id, ImVec2(ImGui::GetContentRegionAvail().x, 5.0f));
                if (ImGui::BeginDragDropTarget()) {
                    ImGuiDragDropFlags targetFlags = 0;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_NAME", targetFlags)) 
                    {
                        SceneGraphNode* draggedNode = *(SceneGraphNode**)payload->Data;
                        SceneGraphNode* moveTo = node;
                        scene->insertNodeAfter(draggedNode, moveTo);
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopStyleColor(2);
            }
        }

        if (!leaf)
        {
            float indentWidth = 10.0f;
            const auto& children = node->getChildren();
            for (auto nodeChild : children)
            {
                if (nodeChild)
                {
                    ImGui::Indent(indentWidth);
                    DrawSceneGraphNode(nodeChild, nodeOpen, destroyEntity, scene);
                    ImGui::Unindent(indentWidth);
                }
            }
        }

        if (createGroup)
        {
            scene->AddEmpty(node, false);
        }
        else if (createObject)
		{
			scene->AddEmpty(node, true, objectType);
		}

        if (destroyEntity) { scene->SetSelectedId(node->getParent()->getId()); scene->RemoveSceneGraphNode(node); }
        if (nodeOpen && !leaf) { 
            ImGui::TreePop(); 
        }
	}
}

void Editor::getInspector(Scene* scene)
{
    if (scene != nullptr)
    {
        SceneGraphNode* node = scene->GetSelectedNode();
        if (node != nullptr)
        {
			bool hasChanges = false;
            float avail = ImGui::GetContentRegionAvail().x;
            
            
            char nameBuffer[256];
            std::strncpy(nameBuffer, node->getName().c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer) - 1] = 0;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer)))
            {
				node->rename(std::string(nameBuffer));
			}

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (node->getId() != 0)
            {
                if (ImGui::Button(ICON_LC_TRASH_2 " Delete", ImVec2(avail, 0))) { scene->RemoveSceneGraphNode(node); }


                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Transform
                char* TransformModeNames[] = { "Local Transform", "Global Transform" };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Combo("##Transform", &m_transformMode, TransformModeNames, IM_ARRAYSIZE(TransformModeNames));
                Transform* transform = node->getTransform();
                if (m_transformMode == 0) {
                    glm::vec3 position = transform->getPosition();
                    glm::vec3 rotation = transform->getRotation();
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text(ICON_LC_AXIS_3D " Position");
                    ImGui::PopFont();
                    if (drawFloat3("Position" + std::to_string(node->getId()), position, 0.01f, 0.0f, 0.0f)) {
                        transform->setPosition(position); 
                        hasChanges = true;
                    }

                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text(ICON_LC_ROTATE_3D " Rotation");
                    ImGui::PopFont();
                    if (drawFloat3("Rotation" + std::to_string(node->getId()), rotation, 0.5f, 0.0f, 0.0f)) {
                        transform->setRotation(rotation);
                        hasChanges = true;
                    }
                }
                else {
                    glm::mat4 globalTransform = transform->getWorldTransform();
                    glm::vec3 position = globalTransform[2];
                    glm::vec3 rotation = globalTransform[1];
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text(ICON_LC_AXIS_3D " Position");
                    ImGui::PopFont();
                    if (drawFloat3("GPosition" + std::to_string(node->getId()), position, 0.01f, 0.0f, 0.0f)) {
                        transform->setWorldPosition(position);
                        hasChanges = true;
                    }

                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text(ICON_LC_ROTATE_3D " Rotation");
                    ImGui::PopFont();
                    if (drawFloat3("GRotation" + std::to_string(node->getId()), rotation, 0.5f, 0.0f, 0.0f)) {
                        transform->setWorldRotation(rotation);
                        hasChanges = true;
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                int currentBoolOperation = node->getBoolOperation();
                char* BoolOperationsNames[] = { ICON_LC_SQUARE_PLUS " Union", ICON_LC_SQUARE_SLASH " Intersection", ICON_LC_SQUARE_MINUS " Difference" };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("##Boolean Operation", &currentBoolOperation, BoolOperationsNames, IM_ARRAYSIZE(BoolOperationsNames))) {
                    BoolOperatios operation = static_cast<BoolOperatios>(currentBoolOperation);
                    node->changeBoolOperation(operation);
                    hasChanges = true;
                    scene->needsRecompilation = true;
                }

                ImGui::Spacing();

                float goop = node->getGoop();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                ImGui::Text(ICON_LC_SQUIRCLE " Smoothing");
                ImGui::PopFont();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::DragFloat("##Smoothing", &goop, 0.01f, 0.00f, 100.0f, "%.3f")) {
                    node->setGoop(goop);
                    hasChanges = true;
                }

                ImGui::Spacing();

                float colorGoop = node->getColorGoop();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                ImGui::Text(ICON_LC_PALETTE " Color Smoothing");
                ImGui::PopFont();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::DragFloat("##Color Smoothing", &colorGoop, 0.01f, 0.00f, 10.0f, "%.3f")) {
					node->setColorGoop(colorGoop);
					hasChanges = true;
				}

            }

            if (!node->isGroup())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                ImGui::Text(ICON_LC_PALETTE " Color");
                ImGui::PopFont();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                glm::vec4 color = node->getColor();
                if (ImGui::ColorEdit3("##Color", &color[0])) {
                    node->setColor(color);
                    hasChanges = true;
                }
        
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                Shape* shape = node->getObject()->getComponent<Shape>();
                int currentShapeId = 0;
                bool idFound = false;

                std::vector<char*> shapeNames;
                shapeNames.reserve(NameToTypeMap.size());
                for (const auto& pair : NameToTypeMap) {
                    shapeNames.push_back(const_cast<char*>(pair.first.c_str()));
                    if (!idFound && pair.second != shape->getType()) {
						currentShapeId++;
                    }
					else {
						idFound = true;
					}
                }

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("##Shape", &currentShapeId, shapeNames.data(), shapeNames.size())) {
                    Type newType = NameToTypeMap.find(shapeNames[currentShapeId])->second;
                    shape->setType(newType);
                    hasChanges = true;
                    scene->needsRecompilation = true;
                }

                ImGui::Spacing();

                switch (shape->getType())
                {
                case Type::Sphere: {
					Sphere sphere = shape->shape.sphere;
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Radius");
                    ImGui::PopFont();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::DragFloat("##Radius", &sphere.radius, 0.01f, 0.01f, 100.0f, "%.3f")) {
                        if (sphere.radius < 0.01f) { sphere.radius = 0.01f; }
						shape->shape.sphere = sphere;
                        hasChanges = true;
					}
                    }
					break;
                case Type::Box: {
                    Box box = shape->shape.box;
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Size");
                    ImGui::PopFont();
                    if (drawFloat3("Size" + std::to_string(node->getId()), box.size, 0.01f, 0.001f, 100.0f)) {
                        if (box.size.x < 0.01f) { box.size.x = 0.01f; }
                        if (box.size.y < 0.01f) { box.size.y = 0.01f; }
                        if (box.size.z < 0.01f) { box.size.z = 0.01f; }
                        shape->shape.box = box;
                        hasChanges = true;
                    }
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Rounding");
                    ImGui::PopFont();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::DragFloat("##Rounding", &box.cornerRadius, 0.01f, 0.0f, 100.0f, "%.3f")) {
                        if (box.cornerRadius < 0.0f) { box.cornerRadius = 0.0f; }
						shape->shape.box = box;
                        hasChanges = true;
					}
                    }
                    break;
                case Type::Cone: {  
                    Cone cone = shape->shape.cone;
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Height");
                    ImGui::PopFont();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::DragFloat("##Height", &cone.height, 0.01f, 0.01f, 100.0f, "%.3f")) {
                        if (cone.height < 0.01f) { cone.height = 0.01f; }
						shape->shape.cone = cone;
                        hasChanges = true;
					}
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Top Radius");
                    ImGui::PopFont();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::DragFloat("##topRadius", &cone.topRadius, 0.01f, 0.0, 70.0, "%.3f")) {
                        if (cone.topRadius < 0.0f) { cone.topRadius = 0.0f; }
						else if (cone.topRadius > 70.0f) { cone.topRadius = 70.0f; }
                        shape->shape.cone = cone;
                        hasChanges = true;
                    }
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Bottom Radius");
                    ImGui::PopFont();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::DragFloat("##bottomRadius", &cone.bottomRadius, 0.01f, 0.0, 70.0, "%.3f")) {
                        if (cone.bottomRadius < 0.0f) { cone.bottomRadius = 0.0f; }
                        else if (cone.bottomRadius > 70.0f) { cone.bottomRadius = 70.0f; }
                        shape->shape.cone = cone;
                        hasChanges = true;
                    }
                    }
					break;
                case Type::Cylinder: {
						Cylinder cylinder = shape->shape.cylinder;
						ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
						ImGui::Text("Height");
						ImGui::PopFont();
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::DragFloat("##HeightCyl", &cylinder.height, 0.01f, 0.001f, 100.0f, "%.3f")) {
							if (cylinder.height < 0.001f) { cylinder.height = 0.001f; }
							shape->shape.cylinder = cylinder;
							hasChanges = true;
						}
						ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
						ImGui::Text("Radius");
						ImGui::PopFont();
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::DragFloat("##RadiusCyl", &cylinder.radius, 0.01f, 0.001f, 70.0f, "%.3f")) {
							if (cylinder.radius < 0.001f) { cylinder.radius = 0.001f; }
							else if (cylinder.radius > 70.0f) { cylinder.radius = 70.0f; }
							shape->shape.cylinder = cylinder;
							hasChanges = true;
						}
					}
                    break;
                case Type::Pyramid: {
                    Pyramid pyramid = shape->shape.pyramid;
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Height");
                    ImGui::PopFont();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::DragFloat("##HeightPyramid", &pyramid.height, 0.01f, 0.001f, 100.0f, "%.3f")) {
						if (pyramid.height < 0.001f) { pyramid.height = 0.001f; }
						shape->shape.pyramid = pyramid;
						hasChanges = true;
                    }
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    ImGui::Text("Base Size");
                    ImGui::PopFont();
                    if (ImGui::DragFloat("##BasePyramid", &pyramid.base, 0.01f, 0.001f, 100.0f, "%.3f")) {
                        if (pyramid.base < 0.001f) { pyramid.base = 0.001f; }
                        shape->shape.pyramid = pyramid;
                        hasChanges = true;
                    }
                    }
                    break;
                case Type::Torus: {
                        Torus torus = shape->shape.torus;
                        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                        ImGui::Text("Radius");
                        ImGui::PopFont();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::DragFloat("##RadiusTorus", &torus.majorRadius, 0.01f, 0.001f, 100.0f, "%.3f")) {
							if (torus.majorRadius < 0.001f) { torus.majorRadius = 0.001f; }
                            if (torus.majorRadius <= torus.minorRadius) { torus.minorRadius = torus.majorRadius - 0.001f; }
							shape->shape.torus = torus;
							hasChanges = true;
						}

                        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
						ImGui::Text("Width");
						ImGui::PopFont();
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::DragFloat("##InnerRadiusTorus", &torus.minorRadius, 0.01f, 0.001f, 100.0f, "%.3f")) {
                            if (torus.minorRadius < 0.001f) { torus.minorRadius = 0.001f; }
                            if (torus.minorRadius >= torus.majorRadius) { torus.majorRadius = torus.minorRadius + 0.001f; }
                            shape->shape.torus = torus;
                            hasChanges = true;
                        }
                    }
					break;
                default:
                    break;
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                ImGui::Text(ICON_LC_UNFOLD_HORIZONTAL " Mirror");
                ImGui::PopFont();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Text("X");
                ImGui::SameLine();
                bool mirrorX = node->getMirrorX();
                if (ImGui::Checkbox("##XMirror", &mirrorX)) { node->setMirrorX(mirrorX); hasChanges = true; scene->needsRecompilation = true; }
                ImGui::SameLine();
                ImGui::Text("Y");
                ImGui::SameLine();
                bool mirrorY = node->getMirrorY();
                if (ImGui::Checkbox("##YMirror", &mirrorY)) { node->setMirrorY(mirrorY);  hasChanges = true; scene->needsRecompilation = true; }
                ImGui::SameLine();
                ImGui::Text("Z");
                ImGui::SameLine();
                bool mirrorZ = node->getMirrorZ();
                if (ImGui::Checkbox("##ZMirror", &mirrorZ)) { node->setMirrorZ(mirrorZ); hasChanges = true; scene->needsRecompilation = true;}

            }
            if (hasChanges) scene->performAction(scene->CreateSnapshot(false));
		}
    }
}

void Editor::getSettings(Scene* scene)
{
	if (scene != nullptr)
	{
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_PALETTE " Background Color");
        ImGui::PopFont();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        glm::vec4 color = scene->getBackgroundColor();
        if (ImGui::ColorEdit3("##BackgroundColor", &color[0])) {
            scene->setBackgroundColor(color);
        }

        ImGui::Spacing();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_SUN " Light-Direction");
        ImGui::PopFont();
        glm::vec3 sunPos = scene->getSunPosition();
        if (drawFloat3("##SunPosition", sunPos, 0.01f, -1.0f, 1.0f))
        {
			scene->setSunPosition(glm::vec4(sunPos.x, sunPos.y, sunPos.z, 0.0f));
		}

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_MINUS " Outline Thickness");
        ImGui::PopFont();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        float outlineThickness = scene->getOutlineThickness();
        if (ImGui::DragFloat("##OutlineThickness", &outlineThickness, 0.01f, 0.0f, 1.0f, "%.3f")) {
			scene->setOutlineThickness(outlineThickness);
		}

		ImGui::Spacing();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_PALETTE " Outline Color");
        ImGui::PopFont();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        glm::vec4 outlineColor = scene->getOutlineColor();
		if (ImGui::ColorEdit3("##OutlineColor", &outlineColor[0])) {
			scene->setOutlineColor(outlineColor);
		}

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_VIDEO " Camera Position");
        ImGui::PopFont();
        glm::vec3 camPos = scene->m_camera.getPosition();
        if (drawFloat3("##CamPosition", camPos, 0.1f, -100.0f, 100.0f))
        {
			scene->setCameraPosition(camPos);
		}

        ImGui::Spacing();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_VIDEO " Camera Look at");
        ImGui::PopFont();
        glm::vec3 camTarget = scene->m_camera.getTarget();
        if (drawFloat3("##CamTarget", camTarget, 0.1f, -100.0f, 100.0f))
        {
            scene->setCameraTarget(camTarget);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_MINUS " Anti-Aliasing");
        ImGui::PopFont();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        int AA = scene->getAA();
        if (ImGui::SliderInt("##Anti-Aliasing", &AA, 1, 4)) {
            scene->setAA(AA);
        }

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::Text(ICON_LC_GRID_3X3 " Grid");
        ImGui::PopFont();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        int showGrid = scene->getShowGrid();
        bool showGridBool = showGrid;
        if (ImGui::Checkbox("##ShowGrid", &showGridBool)) {
            scene->showGrid(int(showGridBool));
        }

	}
}

void Editor::getCode(Scene* scene)
{
    if (scene != nullptr)
    {
        bool hasChanges = false;
        SceneGraphNode* node = scene->GetSelectedNode();
        if (scene->GetSelectedId() <= 0 || scene->GetSelectedNode()->isGroup()) {
            m_editor.SetReadOnly(true);
            m_editor.SetText(m_defaultCode);
            m_selectedCode = -1;
        }
        else {
            m_editor.SetReadOnly(false);
            if (scene->GetSelectedId() != m_selectedCode) {
                Shape* shape = node->getObject()->getComponent<Shape>();
                m_editor.SetText(scene->getShaderByName(shape->getShaderName(), shape->getType()));
                m_newFunctionName = shape->getShaderName()+"New";
            }
            m_selectedCode = scene->GetSelectedId();
        }
        bool clicked = false;
        if (scene->GetSelectedId() > 0 && !scene->GetSelectedNode()->isGroup()) {
            if (ImGui::Button(ICON_LC_CIRCLE_PLAY " Run")) {
                clicked = true;
		    }
            Shape* shape = node->getObject()->getComponent<Shape>();
            float avail = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(avail * 0.8);
            int currentFuncId = 0;
            bool idFound = false;

            std::vector<char*> functionNames;
            std::vector<ShaderShape> shapes = scene->m_shapes[shape->getType()];
            for (const auto& pair : scene->m_shapes[shape->getType()]) {
                functionNames.push_back(const_cast<char*>(pair.name.c_str()));
                if (!idFound && pair.name != shape->getShaderName()) {
                    currentFuncId++;
                }
                else {
                    idFound = true;
                }
            }

            ImGui::SameLine(avail * 0.8 + ImGui::GetStyle().ItemSpacing.x);
            ImGui::SetNextItemWidth(avail * 0.2);
            if (ImGui::Combo("##FunctionNameSelect", &currentFuncId, functionNames.data(), functionNames.size())) {
                shape->setShaderName(functionNames[currentFuncId]);
                m_editor.SetText(scene->getShaderByName(functionNames[currentFuncId], shape->getType()));
                hasChanges = true;
                scene->needsRecompilation = true;
            }

            ImGui::Checkbox("Auto Compile", &m_autoCompile);
            
            avail = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(avail * 0.6);
            
            char nameBuffer[100];
            std::strncpy(nameBuffer, m_newFunctionName.c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer) - 1] = 0;
            ImGui::SetNextItemWidth(avail * 0.2);
            if (ImGui::InputText("##FunctionName", nameBuffer, sizeof(nameBuffer))) {
                m_newFunctionName = std::string(nameBuffer);
            }

            bool nameAlreadyExists = (std::find(functionNames.begin(), functionNames.end(), m_newFunctionName) != functionNames.end());

            ImGui::SameLine();

            if (nameAlreadyExists || m_newFunctionName.length() < 4) ImGui::BeginDisabled();
            if (ImGui::Button(ICON_LC_CIRCLE_PLUS " New Function", ImVec2(avail * 0.2, 0.0))) {
                std::string oldFunctionName = shape->getShaderName();
                std::string newCode = m_editor.GetText();
                size_t pos = newCode.find(oldFunctionName);
                if (pos != std::string::npos) {
                    newCode.replace(pos, oldFunctionName.length(), m_newFunctionName);
                }
                m_editor.SetText(newCode);
                scene->AddShape(m_newFunctionName, newCode, shape->getType());
                shape->setShaderName(m_newFunctionName);
                scene->needsRecompilation = true;
            }
            if (nameAlreadyExists || m_newFunctionName.length() < 4) ImGui::EndDisabled();
        }

        if (!m_errors.empty()) {
            TextEditor::ErrorMarkers markers;
            for (const auto& error : m_errors) {
                markers[error.line - m_shaderCodeLinesOffset] = error.message;
            }
            m_editor.SetErrorMarkers(markers);
        }
        else {
			m_editor.SetErrorMarkers({});
		}

		m_editor.Render("Code Editor");

        if (m_editor.IsTextChanged())
        {
            char* error = nullptr;
            vkUtil::compileShaderSourceToSpirv(m_editor.GetText(), "", GLSLANG_STAGE_COMPUTE, true, &error);
            if (error != nullptr) {
                std::string errorString = error;
                m_errors = parseGlslErrors(error);
            }
            else {
				m_errors.clear();
                if (m_autoCompile) {
					SceneGraphNode* node = scene->GetSelectedNode();
					Shape* shape = node->getObject()->getComponent<Shape>();
					scene->setShaderByName(shape->getShaderName(), m_editor.GetText(), shape->getType());
					scene->needsRecompilation = true;
				}
			}
		}
        if (clicked && m_errors.empty() && scene->GetSelectedId() > 0) {
            SceneGraphNode* node = scene->GetSelectedNode();
            Shape* shape = node->getObject()->getComponent<Shape>();
            std::string s = m_editor.GetText();
            scene->setShaderByName(shape->getShaderName(), m_editor.GetText(), shape->getType());
            scene->needsRecompilation = true;
        }
        if (hasChanges) scene->performAction(scene->CreateSnapshot(false));
	}
}

void Editor::SettingsPanel(Scene* scene)
{
    auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin(ICON_LC_LAYERS_3 " Scene", nullptr, flags);
    if (ImGui::IsWindowDocked())
    {
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        m_maskScene = glm::vec4(pos.x, pos.y, size.x, size.y);
    }
    else 
    {
    	m_maskScene = glm::vec4(0.0f);
    }
    getScene(scene);
    ImGui::End();
    //--------------------------------------------------------------------------
    ImGui::Begin(ICON_LC_SETTINGS " Scene Settings", nullptr, flags);
    if (ImGui::IsWindowDocked())
    {
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        m_maskScene = glm::vec4(pos.x, pos.y, size.x, size.y);
    }
    else
    {
        m_maskScene = glm::vec4(0.0f);
    }
    getSettings(scene);
    ImGui::End();
    //--------------------------------------------------------------------------
    ImGui::Begin(ICON_LC_CODE_XML " Code", nullptr, flags);
    if (ImGui::IsWindowDocked())
    {
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        m_maskCode = glm::vec4(pos.x, pos.y, size.x, size.y);
    }
    else
    {
        m_maskCode = glm::vec4(0.0f);
    }
    getCode(scene);
    ImGui::End();
    //--------------------------------------------------------------------------
    ImGui::Begin(ICON_LC_TEXT_CURSOR_INPUT" Inspector", nullptr, flags);
    if (ImGui::IsWindowDocked())
    {
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        m_maskInspector = glm::vec4(pos.x, pos.y, size.x, size.y);
    }
    else
    {
        m_maskInspector = glm::vec4(0.0f);
    }
    getInspector(scene);
    ImGui::End();
}

void Editor::MenuBar(Scene* scene)
{
    if (ImGui::BeginMainMenuBar())
    {
        // Populate your menu bar here
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::Selectable(ICON_LC_FILE_PLUS " New Scene"))
            {
                ImGui::CloseCurrentPopup();
				scene->newScene();
            }
            if (ImGui::Selectable(ICON_LC_FOLDER_OPEN_DOT " Open Scene"))
            {
                ImGui::CloseCurrentPopup();
                openScene(scene);
            }
            if (ImGui::Selectable(ICON_LC_SAVE " Save Scene"))
            {
                ImGui::CloseCurrentPopup();
                saveScene(scene);

            }
            if (ImGui::Selectable(ICON_LC_SAVE_ALL " Save Scene As"))
            {
                ImGui::CloseCurrentPopup();
                saveScene(scene, true);

            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) 
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            ImGui::Text(ICON_LC_VIDEO " Camera Move Speed");
            ImGui::PopFont();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::SliderFloat("##CamSpeed", &scene->m_cameraSpeed, 0.1f, 2.0f))
            {
            }

            ImGui::Spacing();

            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            ImGui::Text(ICON_LC_ROTATE_3D " Camera Orbit Speed");
            ImGui::PopFont();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::SliderFloat("##CamOrbSpeed", &scene->m_orbitSpeed, 0.1f, 2.0f))
            {
            }
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Render"))
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            ImGui::Text(ICON_LC_FRAME " Resolution");
            ImGui::PopFont();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::DragInt2("##Resolution", &m_imageSize.x , 1.0f, 1, 16384))
            {
				if (m_imageSize.x < 1) m_imageSize.x = 1;
                if (m_imageSize.y < 1) m_imageSize.y = 1;
                if (m_imageSize.x > 16384) m_imageSize.x = 16384;
                if (m_imageSize.y > 16384) m_imageSize.y = 16384;
			}

            ImGui::Spacing();

            if (ImGui::Selectable(ICON_LC_IMAGE" Render Image"))
            {
                ImGui::CloseCurrentPopup();
                m_renderImage = true;
            }
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Keybindings"))
        {
            if (ImGui::BeginTable("KeybindingsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                // Setup columns
                ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);

                // Table headers
                ImGui::TableHeadersRow();

                // Populate the table with keybindings
                for (const Keybinding& kb : keybindings)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", kb.key);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", kb.description);
                }

                // End the table
                ImGui::EndTable();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    m_menuBarHeight = ImGui::GetFrameHeight();
}

void Editor::openAddPanel(ImGuiContext* context, Scene* scene)
{
    std::cout << "Open Add Panel" << std::endl;
    ImGui::SetCurrentContext(context);

    // Get the current mouse cursor position
    ImVec2 mousePos = ImGui::GetMousePos();

    // Set the position of the next window to the cursor position
    ImGui::SetNextWindowPos(mousePos, ImGuiCond_Appearing);

    // Open the popup
    ImGui::OpenPopup("AddMenu");

}

void Editor::AddPanel(Scene* scene) {

    if (ImGui::BeginPopup("AddMenu"))
    {
        if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Selectable(ICON_LC_BOXES " Group"))
        {
            ImGui::CloseCurrentPopup();
            scene->AddEmpty(scene->GetSelectedNode());
        }
        for (const auto& object : NameToTypeMap)
        {
            if (ImGui::Selectable(object.first.c_str()))
            {
				ImGui::CloseCurrentPopup();
				scene->AddEmpty(scene->GetSelectedNode(), true, object.second);
			}
		}

        ImGui::EndPopup();
    }
}

void Editor::Gizmo(Scene* scene)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(); 

    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

    if (scene->GetSelectedId() != 0) { 
        ImGuizmo::MODE mode = (m_transformMode == 0) ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;
        glm::mat4 trans = scene->GetSelectedNode()->getTransform()->getWorldTransform();
        glm::vec3 position = trans[2];
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 viewMatrix = scene->m_camera.getViewMatrix();
        glm::mat4 projMatrix = scene->m_camera.getProjectionMatrix();
        float tmpPos[16];
        float translation[3], rotation[3], scale[3];
        ImGuizmo::RecomposeMatrixFromComponents(&position.x, &trans[1].x, &trans[0].x, tmpPos);
        ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix),
            ImGuizmo::OPERATION::TRANSLATE, mode, tmpPos);
        ImGuizmo::DecomposeMatrixToComponents(tmpPos, translation, rotation, scale);
        glm::vec3 p = glm::vec3(translation[0], translation[1], translation[2]);
        if (p != position) {
            scene->GetSelectedNode()->getTransform()->setWorldPosition(p);
            scene->performAction(scene->CreateSnapshot(false));
        }
        
        float tmp[16];
        glm::vec3 rot = trans[1];
        ImGuizmo::RecomposeMatrixFromComponents(&position.x, &rot.x, &trans[0].x, tmp);
        ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix),
            ImGuizmo::OPERATION::ROTATE, mode, tmp); 
        ImGuizmo::DecomposeMatrixToComponents(tmp, translation, rotation, scale);
        glm::vec3 r = glm::vec3(rotation[0], rotation[1], rotation[2]);
        if (r != rot) {
            scene->GetSelectedNode()->getTransform()->setWorldRotation(r);
                scene->performAction(scene->CreateSnapshot(false));
        }
    }
    ImGui::End();
}

void Editor::saveScene(Scene* scene, bool saveAs) {
    std::string filename = scene->getFilename();
    if (saveAs || filename == "" || !hasCorrectExtension(filename, ".sym")) {
        const char* filterPatterns[] = { "*.sym" };
        const char* saveFilePath = tinyfd_saveFileDialog(
            "Save Scene",
            (scene->GetSceneGraph()->getName() + ".sym").c_str(),
            1,
            filterPatterns,
            NULL
        );
        if (saveFilePath) {
            filename = saveFilePath;
        }
    }

    if (filename != "") {
        if (filename.find(".sym") == std::string::npos) {
            filename += ".sym";
        }
        scene->saveScene(filename);
    }
}

void Editor::openScene(Scene* scene) {
    const char* filterPatterns[] = { "*.sym" };
    const char* loadFilePath = tinyfd_openFileDialog(
        "Open Scene",
        "",
        1,
        filterPatterns,
        NULL,
        0
    );

    if (loadFilePath) {
        std::string filename(loadFilePath);
        if (hasCorrectExtension(loadFilePath, ".sym")) {
            scene->loadScene(loadFilePath);
        }
        else {
            tinyfd_messageBox("Error", "File is not a .sym file", "ok", "error", 1);
        }
    }
}

void  Editor::setTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.5000000014901161f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 10.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(30.0f, 30.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ChildRounding = 5.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 10.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(5.0f, 3.5f);
    style.FrameRounding = 5.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(5.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(5.0f, 5.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 5.0f;
    style.ColumnsMinSpacing = 5.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 15.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.3605149984359741f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1490196138620377f, 0.1137254908680916f, 0.2705882489681244f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1490196138620377f, 0.1137254908680916f, 0.2705882489681244f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2313725501298904f, 0.1764705926179886f, 0.4196078479290009f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3803921639919281f, 0.4235294163227081f, 0.572549045085907f, 0.5490196347236633f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1176470592617989f, 0.09019608050584793f, 0.2196078449487686f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1176470592617989f, 0.09019608050584793f, 0.2196078449487686f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2588235437870026f, 0.2588235437870026f, 0.2588235437870026f, 0.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 0.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.596260130405426f, 0.4358893930912018f, 0.995708167552948f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5960784554481506f, 0.43529412150383f, 0.9960784316062927f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.4490196347236633f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.4490196347236633f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.4490196347236633f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6695278882980347f, 0.6695212125778198f, 0.6695212125778198f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.2901960909366608f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(9.999999974752427e-07f, 9.999899930335232e-07f, 9.999899930335232e-07f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.362287163734436f, 0.2545267045497894f, 0.6309012770652771f, 0.5490196347236633f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}