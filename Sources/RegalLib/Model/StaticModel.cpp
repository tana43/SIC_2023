#include "StaticModel.h"
#include "../Graphics/Graphics.h"
#include "../../../External/imgui/imgui.h"

namespace Regal::Model
{
    StaticModel::StaticModel(const char* fbxFilename, bool traiangulate)
    {
        skinnedMesh = std::make_unique<Regal::Resource::SkinnedMesh>(
            Regal::Graphics::Graphics::Instance().GetDevice()
            ,fbxFilename,traiangulate);
    }

    void StaticModel::Render()
    {
        skinnedMesh->_Render(Regal::Graphics::Graphics::Instance().GetDeviceContext(),&transform);
    }

    void StaticModel::DrawDebug()
    {
        //if (ImGui::BeginMenu("Model"))
        if (ImGui::TreeNode("Model"))
        {
            skinnedMesh->DrawDebug();
            transform.DrawDebug();
            ImGui::TreePop();
            //ImGui::EndMenu();
        }
    }
}