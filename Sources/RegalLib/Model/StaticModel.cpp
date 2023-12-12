#include "StaticModel.h"
#include "../Graphics/Graphics.h"
#include "../../../External/imgui/imgui.h"
#include "../Resource/ResourceManager.h"

namespace Regal::Model
{
    StaticModel::StaticModel(const char* fbxFilename, bool traiangulate)
    {
        
        skinnedMesh = ResourceManager::Instance().LoadModelResource(
            Regal::Graphics::Graphics::Instance().GetDevice(), fbxFilename, traiangulate
        );
    }

    void StaticModel::Render()
    {
        skinnedMesh->_Render(Regal::Graphics::Graphics::Instance().GetDeviceContext(),transform,emissiveIntencity,emissiveColor);
    }

    void StaticModel::DrawDebug()
    {
        //if (ImGui::BeginMenu("Model"))
        if (ImGui::TreeNode("Model"))
        {
            skinnedMesh->DrawDebug();
            ImGui::ColorEdit4("Emissive Color", &emissiveColor.x);
            ImGui::SliderFloat("Emissive Intencity", &emissiveIntencity, 0.0f, 10.0f);
            transform.DrawDebug();
            ImGui::TreePop();
            //ImGui::EndMenu();
        }
    }
}