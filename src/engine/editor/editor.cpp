#pragma once
#include "Editor.h"
#include "glm/gtc/type_ptr.hpp"

Editor::Editor()
{
    m_maskScene = glm::vec4(0.0f);
    m_maskCode = glm::vec4(0.0f);
    m_maskInspector = glm::vec4(0.0f);
}

Editor::~Editor()
{
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
            auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.22f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

            ImGui::DockBuilderDockWindow("Code", dock_id_down);
            ImGui::DockBuilderDockWindow("Scene", dock_id_left);
            ImGui::DockBuilderDockWindow("Lighting", dock_id_left);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Settings", dock_id_right);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }
}

void Editor::getScene(Scene* scene)
{
	if (scene != nullptr)
	{
        SceneGraphNode* sceneGraph = scene->GetSceneGraph();
        DrawSceneGraphNode(sceneGraph, true, false, scene);
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

        const bool leaf = node->isLeaf();

        if (draw)
        {
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
            if (!leaf) { nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow; }
            else { nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; }

            if (id == selectedId) { nodeFlags |= ImGuiTreeNodeFlags_Selected; }

            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            nodeOpen =
                ImGui::TreeNodeEx((void*)(intptr_t)id, nodeFlags, "%s", node->getName().c_str());
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
                std::vector<std::string> menuItems = {};
                if (isGroup) {
                    menuItems.push_back("Add Group");
                	menuItems.push_back("Add Object");
                }
                if (id != 0)
                {
                    menuItems.push_back("Delete Object");
                }

                for (size_t i = 0; i < menuItems.size(); i++)
                {
                    const auto& itemName = menuItems[i];
                    if (ImGui::Selectable(itemName.c_str()))
                    {
                        if (i == 0)
                        {
                            createGroup = true;
                        }
						else if (i == 1)
						{
							createObject = true;
						}
                        else if (id != 0 && i == 2)
                        {
                            destroyEntity = true;
                        }
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
        }

        if (!leaf)
        {
            const auto& children = node->getChildren();
            for (auto nodeChild : children)
            {
                if (nodeChild)
                {
                    DrawSceneGraphNode(nodeChild, nodeOpen, destroyEntity, scene);
                }
            }
        }

        if (createGroup)
        {
            scene->AddEmpty(node, false);
        }
        else if (createObject)
		{
			scene->AddEmpty(node, true);
		}

        if (destroyEntity) { scene->SetSelectedId(node->getParent()->getId()); scene->RemoveSceneGraphNode(node); }
        if (nodeOpen && !leaf) { ImGui::TreePop(); }
	}
}

void Editor::getInspector(Scene* scene)
{
    if (scene != nullptr)
    {
        SceneGraphNode* node = scene->GetSelectedNode();
        if (node != nullptr)
        {
			ImGui::Begin("Inspector");
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

            // Transform
            Transform* transform = node->getTransform();
            if (ImGui::CollapsingHeader("Local Transform")) {
                glm::vec3 position = transform->getPosition();
                glm::vec3 rotation = transform->getRotation();
                glm::vec3 scale = transform->getScale();
                ImGui::Text("Position"); ImGui::SameLine();
                if (ImGui::InputFloat3("##Position", &position[0])) {
                    transform->setPosition(position);
                }
                ImGui::Text("Rotation"); ImGui::SameLine();
                if (ImGui::InputFloat3("##Rotation", &rotation[0])) {
                    transform->setRotation(rotation);
                }
            }

            if (ImGui::CollapsingHeader("Global Transform")) {
                glm::mat4 globalTransform = transform->getWorldTransform();
                glm::vec3 position = globalTransform[2];
                glm::vec3 rotation = globalTransform[1];
                glm::vec3 scale = globalTransform[0];
                ImGui::Text("Position"); ImGui::SameLine();
                if (ImGui::InputFloat3("##GPosition", &position[0])) {
                    transform->setWorldPosition(position);
                }
                ImGui::Text("Rotation"); ImGui::SameLine();
                if (ImGui::InputFloat3("##GRotation", &rotation[0])) {
                    transform->setWorldRotation(rotation);
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (node->isGroup())
			{
                float buttonWidth = avail * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f;
				if (ImGui::Button("Add Group", ImVec2(buttonWidth, 0))){scene->AddEmpty(node, false);}
                ImGui::SameLine();
				if (ImGui::Button("Add Object", ImVec2(buttonWidth, 0))) {scene->AddEmpty(node, true);}
			}
            if (node->getId() != 0)
			{
                if (ImGui::Button("Delete", ImVec2(avail, 0))) {scene->RemoveSceneGraphNode(node);}
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (node->isGroup())
            {
                int currentBoolOperation = node->getBoolOperation();
                char* BoolOperationsNames[] = { "Union", "Intersection", "Difference" };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("##Boolean Operation", &currentBoolOperation, BoolOperationsNames, IM_ARRAYSIZE(BoolOperationsNames))) {
                    BoolOperatios operation = static_cast<BoolOperatios>(currentBoolOperation);
                    node->changeBoolOperation(operation);
                }
			}
            else 
            {
                Shape* shape = node->getObject()->getComponent<Shape>();
                int currentShapeId = static_cast<int>(shape->getType());
                char* ShapeNames[] = { "Spehre", "Box", "Cone" };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("##Shape", &currentShapeId, ShapeNames, IM_ARRAYSIZE(ShapeNames))) {
                    Type newType = static_cast<Type>(currentShapeId);
                    shape->setType(newType);
                }

                ImGui::Spacing();

                switch (shape->getType())
                {
                case Type::Sphere: {
					Sphere sphere = shape->shape.sphere;
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25);
                    ImGui::Text("Radius"); ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputFloat("##Radius", &sphere.radius)) {
						shape->shape.sphere = sphere;
					}
                    }
					break;
                case Type::Box: {
                    Box box = shape->shape.box;
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25);
                    ImGui::Text("Size"); ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputFloat3("##Size", &box.size[0])) {
						shape->shape.box = box;
					}
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25);
                    ImGui::Text("Rounding"); ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputFloat("##Rounding", &box.cornerRadius)) {
                        shape->shape.box = box;
                    }
                    }
                    break;
                case Type::Cone: {
                    Cone cone = shape->shape.cone;
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25);
                    ImGui::Text("Height"); ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputFloat("##Height", &cone.height)) {
						shape->shape.cone = cone;
					}
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25);
                    ImGui::Text("Angle"); ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputFloat("##Angle", &cone.angle)) { 
                        shape->shape.cone = cone;
                    }
                    }
					break;
                default:
                    break;
                }

            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("DEBUG");
            ImGui::Text("ID: %d", node->getId());
            ImGui::SameLine();
            ImGui::Text("IDScn: %d", node->getDataId());
            if (node->getParent() != nullptr)
            {
                ImGui::Text("Parent: %s", node->getParent()->getName().c_str());
            }
            else
            {
                ImGui::Text("Parent: None");
            }
            ImGui::Text("Children: %d", node->getChildren().size());
            if (node->getChildren().size() > 0) {
                ImGui::Text("CHild id: %d", node->getData()->data0.z);
                ImGui::Text("Children: ");
				for (auto child : node->getChildren())
				{
					ImGui::SameLine();
					ImGui::Text("%s", child->getName().c_str());
				}
            }
            ImGui::Text("IsGroup: %d", node->isGroup());
     
			ImGui::End();
		}
    }
}

void Editor::SettingsPanel(Scene* scene)
{
    auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Scene", nullptr, flags);
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
    ImGui::Begin("Lighting", nullptr, flags);
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
    ImGui::Text("Hello, Lighting!");
    std::array<NodeData, 50> nodeData = scene->GetNodeData();
    for (int i = 0; i < 10; i++)
    {
        glm::vec3 position = nodeData[i].transform[2];
		ImGui::InputInt("ID", &nodeData[i].data0.x);
        ImGui::InputInt("realID", &nodeData[i].data1.z);
		ImGui::InputInt("Parent", &nodeData[i].data0.y);
		ImGui::InputInt("FirstChild:", &nodeData[i].data0.z);
        ImGui::InputInt("ChildCount:", &nodeData[i].data0.w);
		ImGui::InputInt("IsGroup:", &nodeData[i].data1.y);
        ImGui::InputFloat3("##pos", &position[0]);
        ImGui::Text("Data-----------------------------");
    }
    ImGui::End();
    //--------------------------------------------------------------------------
    ImGui::Begin("Code", nullptr, flags);
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
    ImGui::Text("Hello, Code!");
    ImGui::End();
    //--------------------------------------------------------------------------
    ImGui::Begin("Settings", nullptr, flags);
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
    ImGui::Text("Hello, Settings!");
    ImGui::End();
    //--------------------------------------------------------------------------
    ImGui::Begin("Inspector", nullptr, flags);
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

void Editor::MenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        // Populate your menu bar here
        if (ImGui::BeginMenu("Test"))
        {
            // Menu items
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    m_menuBarHeight = ImGui::GetFrameHeight();
}

void Editor::Gizmo(Scene* scene)
{
    // No need to reapply style inside the window, just set up once before ImGui::Begin()
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

    if (scene->GetSelectedId() != 0) { 
        glm::mat4 trans = scene->GetSelectedNode()->getTransform()->getWorldTransform();
        glm::vec3 position = trans[2];
        //position *= 0.01f;
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 viewMatrix = scene->m_camera.getViewMatrix();
        glm::mat4 projMatrix = scene->m_camera.getProjectionMatrix();
        ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix),
            ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, glm::value_ptr(transform));
        scene->GetSelectedNode()->getTransform()->setWorldPosition(glm::vec3(transform[3]));
    }
    ImGui::End();
}


void  Editor::setTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.1000000014901161f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 10.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(30.0f, 30.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ChildRounding = 5.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 10.0f;
    style.PopupBorderSize = 0.0f;
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
    style.Colors[ImGuiCol_Tab] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.5490196347236633f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.501960813999176f, 0.3019607961177826f, 0.5490196347236633f, 0.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.501960813999176f, 0.3019607961177826f, 1.0f, 0.0f);
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