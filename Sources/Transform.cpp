#include "Transform.h"
#include "../imgui/imgui.h"
#include <vector>


void Transform::DrawDebug()
{
    if (ImGui::TreeNode("Transform"))
    {
        ImGui::DragFloat3("Position", &position.x, 0.1f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat3("Scale", &scale.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat3("Rotation", &rotation.x, 0.01f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat("ScaleFactor", &scaleFactor, 0.01f, 0.01f, 100.0f);

        ImGuiCoordinateComboUI();

        ImGui::TreePop();
    }
}

DirectX::XMMATRIX Transform::CalcWorldMatrix()
{
    return DirectX::XMMATRIX();
}

bool Transform::ImGuiCoordinateComboUI()
{
    bool changed = false;

    if (ImGui::BeginCombo("Coordinate System", coordinateSystemName[coordinateSystem].c_str())) {
        for (int n = 0; n < MAX_COORDINATE_SYSTEM; n++) {
            bool isSelected = (coordinateSystem == n);
            if (ImGui::Selectable(coordinateSystemName[n].c_str(), isSelected)) {
                coordinateSystem = n;
                changed = true;
            }
            if (isSelected) {
                // Set the initial focus when opening the combo (scrolling + for
                // keyboard navigation support in the upcoming navigation branch)
               // ImGui::SetItemDefaultFertexocus();
            }
        }
        ImGui::EndCombo();
    }

    return changed;
}
