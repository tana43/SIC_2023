#include "SkinnedMesh.h"
#include "misc.h"

#include <sstream>
#include <functional>

SkinnedMesh::SkinnedMesh(ID3D11Device* device, const char* fbxFilename, bool triangulate)
{
    FbxManager* fbxManager{ FbxManager::Create() };
    FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

    FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
    bool importStatus{ false };
    importStatus = fbxImporter->Initialize(fbxFilename);
    _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());  

    importStatus = fbxImporter->Import(fbxScene);
    _ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

    FbxGeometryConverter fbxConverter(fbxManager);
    if (triangulate)
    {
        fbxConverter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
        fbxConverter.RemoveBadPolygonsFromMeshes(fbxScene);
    }

    std::function<void(FbxNode*)> traverse{[&](FbxNode* fbxNode) {
        Scene::Node& node{sceneView.nodes.emplace_back()};
        node.attribute = fbxNode->GetNodeAttribute() ?
            fbxNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
        node.name = fbxNode->GetName();
        node.uniqueId = fbxNode->GetUniqueID();
        node.parentIndex = sceneView.indexOf(fbxNode->GetParent() ?
            fbxNode->GetParent()->GetUniqueID() : 0);
        for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
        {
            traverse(fbxNode->GetChild(childIndex));
        }
    } };
    traverse(fbxScene->GetRootNode());

#if 1
    for (const Scene::Node& node : sceneView.nodes)
    {
        FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name.c_str()) };
        //Display node data in the output window as debug
        std::string nodeName = fbxNode->GetName();
        uint64_t uid = fbxNode->GetUniqueID();
        uint64_t parentUid = fbxNode->GetParent() ? fbxNode->GetParent()->GetUniqueID() : 0;
        int32_t type = fbxNode->GetNodeAttribute() ? fbxNode->GetNodeAttribute()->GetAttributeType() : 0;

        std::stringstream debugString;
        debugString << nodeName << ":" << uid << ":" << parentUid << ":" << type << "\n";
        OutputDebugStringA(debugString.str().c_str());
    }
#endif // 1
    fbxManager->Destroy();
}
