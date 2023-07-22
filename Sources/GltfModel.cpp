#include "GltfModel.h"
#define TINYGLTF_IMPLEMENTATION
#include "../External/tinygltf-release/tiny_gltf.h"
#include "misc.h"
#include <stack>
#include "Shader.h"

bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*,
    int, int, const unsigned char*, int, void*)
{
    return true;
}

GltfModel::GltfModel(ID3D11Device* device, const std::string& filename)
{
    tinygltf::TinyGLTF tinyGltf;
    tinyGltf.SetImageLoader(NullLoadImageData, nullptr);

    tinygltf::Model gltfModel;
    std::string error, warning;
    bool succeeded{ false };
    if (filename.find(".glb") != std::string::npos)
    {
        succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
    }
    else if (filename.find(".gltf") != std::string::npos)
    {
        succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
    }

    _ASSERT_EXPR_A(warning.empty(), warning.c_str());
    _ASSERT_EXPR_A(error.empty(), warning.c_str());
    _ASSERT_EXPR_A(succeeded,L"Failed to load gltf file");
    for (std::vector<tinygltf::Scene>::const_reference gltfScene : gltfModel.scenes)
    {
        Scene& scene{ scenes.emplace_back() };
        scene.name = gltfModel.scenes.at(0).name;
        scene.nodes = gltfModel.scenes.at(0).nodes;
    }

    FetchMeshs(device, gltfModel);

    //シェーダーオブジェクトの生成
    const std::map<std::string, BufferView>& vertexBufferViews{
        meshes.at(0).primitives.at(0).vertexBufferViews
    };
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
    {
        {"POSITION",0,vertexBufferViews.at("POSITION").format,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"NORMAL",0,vertexBufferViews.at("NORMAL").format,1,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TANGENT",0,vertexBufferViews.at("TANGENT").format,2,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,vertexBufferViews.at("TEXCOORD").format,3,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"JOINTS",0,vertexBufferViews.at("JOINTS").format,4,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"WEIGHTS",0,vertexBufferViews.at("WEIGHTS").format,5,0,D3D11_INPUT_PER_VERTEX_DATA,0},
    };

    Shader::CreateVSFromCso(device, "./Resources/Shader/GltfModelVS.cso", vertexShader.ReleaseAndGetAddressOf(),
        inputLayout.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    Shader::CreatePSFromCso(device, "./Resources/Shader/GltfModelPS.cso", pixelShader.ReleaseAndGetAddressOf());

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(PrimitiveConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    HRESULT hr;
    hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveCbuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void GltfModel::CumulateTransforms(std::vector<Node>& nodes)
{
    using namespace DirectX;

    std::stack<XMFLOAT4X4> parentGlobalTransforms;
    std::function<void(int)> traverse{[&](int nodeIndex)->void
        {
            Node& node{ nodes.at(nodeIndex) };
            XMMATRIX S{ XMMatrixScaling(node.scale.x,node.scale.y,node.scale.z) };
            XMMATRIX R{ XMMatrixRotationQuaternion(
            XMVectorSet(node.rotation.x,node.rotation.y,node.rotation.z,node.rotation.w)
            ) };
            XMMATRIX T{ XMMatrixTranslation(node.translation.x,node.translation.y,node.translation.z) };
            XMStoreFloat4x4(&node.globalTransform, S * R * T * XMLoadFloat4x4(&parentGlobalTransforms.top()));
            for (int childIndex : node.children)
            {
                parentGlobalTransforms.push(node.globalTransform);
                traverse(childIndex);
                parentGlobalTransforms.pop();
            }
        }
    };
    for (std::vector<int>::value_type nodeIndex : scenes.at(0).nodes)
    {
        parentGlobalTransforms.push({ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 });
        traverse(nodeIndex);
        parentGlobalTransforms.pop();
    }
}

GltfModel::BufferView GltfModel::MakeBufferView(const tinygltf::Accessor& accessor)
{
    BufferView bufferView;
    switch (accessor.type)
    {
    case TINYGLTF_TYPE_SCALAR:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format = DXGI_FORMAT_R16_UINT;
            bufferView.strideInBytes = sizeof(USHORT);
            break;

        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format = DXGI_FORMAT_R32_UINT;
            bufferView.strideInBytes = sizeof(UINT);
            break;

        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;
    case TINYGLTF_TYPE_VEC2:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format = DXGI_FORMAT_R32G32_FLOAT;
            bufferView.strideInBytes = sizeof(FLOAT) * 2;
            break;

        default:
            _ASSERT_EXPR(FALSE, L"This sccessor component type is not supported.");
            break;
        }
        break;

    case TINYGLTF_TYPE_VEC3:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format = DXGI_FORMAT_R32G32B32_FLOAT;
            bufferView.strideInBytes = sizeof(FLOAT) * 3;
            break;

        default:
            _ASSERT_EXPR(FALSE, L"This sccessor component type is not supported.");
            break;
        }
        break;

    case TINYGLTF_TYPE_VEC4:
        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bufferView.format = DXGI_FORMAT_R16G16B16A16_UINT;
            bufferView.strideInBytes = sizeof(USHORT) * 4;
            break;

        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bufferView.format = DXGI_FORMAT_R32G32B32A32_UINT;
            bufferView.strideInBytes = sizeof(UINT) * 4;
            break;

        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            bufferView.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            bufferView.strideInBytes = sizeof(USHORT) * 4;
            break;

        default:
            _ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
            break;
        }
        break;

    default:
        _ASSERT_EXPR(FALSE, L"This accessor type is not supported.");
        break;
    }

    bufferView.sizeInBytes = static_cast<UINT>(accessor.count * bufferView.strideInBytes);
    return bufferView;
}

void GltfModel::FetchMeshs(ID3D11Device* device, const tinygltf::Model& gltfModel)
{
    HRESULT hr;

    for (std::vector<tinygltf::Mesh>::const_reference gltfMesh : gltfModel.meshes)
    {
        Mesh& mesh{ meshes.emplace_back() };
        mesh.name = gltfMesh.name;
        for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
        {
            Mesh::Primitive& primitive{mesh.primitives.emplace_back()};
            primitive.material = gltfPrimitive.material;

            //Create index buffer
            const tinygltf::Accessor& gltfAccessor{gltfModel.accessors.at(gltfPrimitive.indices)};
            const tinygltf::BufferView& gltfBufferView{gltfModel.bufferViews.at(gltfAccessor.bufferView)};

            primitive.indexBufferView = MakeBufferView(gltfAccessor);

            D3D11_BUFFER_DESC bufferDesc{};
            bufferDesc.ByteWidth = static_cast<UINT>(primitive.indexBufferView.sizeInBytes);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA subresourceData{};
            subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferView.buffer).data.data()
                + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
            hr = device->CreateBuffer(&bufferDesc, &subresourceData,
                primitive.indexBufferView.buffer.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

            //Create vertex buffers
            for (std::map<std::string,int>::const_reference gltfAttribute : gltfPrimitive.attributes)
            {
                const tinygltf::Accessor& gltfAccessor{gltfModel.accessors.at(gltfAttribute.second)};
                const tinygltf::BufferView& gltfBufferView{gltfModel.bufferViews.at(gltfAccessor.bufferView)};

                BufferView vertexBufferView{ MakeBufferView(gltfAccessor) };

                D3D11_BUFFER_DESC bufferDesc{};
                bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes);
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                D3D11_SUBRESOURCE_DATA subresourceData{};
                subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferView.buffer).data.data()
                    + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                hr = device->CreateBuffer(&bufferDesc, &subresourceData,
                    vertexBufferView.buffer.ReleaseAndGetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

                primitive.vertexBufferViews.emplace(std::make_pair(gltfAttribute.first, vertexBufferView));
            }

            //Add dummy attributes if and are missing
            const std::unordered_map<std::string, BufferView> attributes{
                {"TANGENT", { DXGI_FORMAT_R32G32B32A32_FLOAT }},
                { "TEXCOORD_0",{DXGI_FORMAT_R32G32_FLOAT} },
                { "JOINTS_0",{DXGI_FORMAT_R16G16B16A16_UINT} },
                { "WEIGHTS_0",{DXGI_FORMAT_R32G32B32A32_FLOAT} },
            };
            for (std::unordered_map<std::string,BufferView>::const_reference attribute : attributes)
            {
                if(primitive.vertexBufferViews.find(attribute.first) == primitive.vertexBufferViews.end())
                {
                    primitive.vertexBufferViews.insert(std::make_pair(attribute.first, attribute.second));
                }
            }
        }
    }
}

void GltfModel::Render(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& world)
{
    using namespace DirectX;

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    std::function<void(int)> Traverse{[&](int nodeIndex)->void {
        const Node& node{ nodes.at(nodeIndex) };
        if (node.mesh > -1)
        {
            const Mesh& mesh{ meshes.at(node.mesh) };
            for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives)
            {
                ID3D11Buffer* vertexBuffers[]{
                    primitive.vertexBufferViews.at("POSITION").buffer.Get(),
                    primitive.vertexBufferViews.at(
                        "NORMAL").buffer.Get(),
                    primitive.vertexBufferViews.at("TANGENT").buffer.Get(),
                    primitive.vertexBufferViews.at("TEXCOODE_0").buffer.Get(),
                    primitive.vertexBufferViews.at("JOINTS_0").buffer.Get(),
                    primitive.vertexBufferViews.at("WEIGHTS_0").buffer.Get(),
                };

                UINT strides[]{
                    static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").strideInBytes),
                    static_cast<UINT>(primitive.vertexBufferViews.at("NORMAL").strideInBytes),
                    static_cast<UINT>(primitive.vertexBufferViews.at("TANGENT").strideInBytes),
                    static_cast<UINT>(primitive.vertexBufferViews.at("TEXCOODE_0").strideInBytes),
                    static_cast<UINT>(primitive.vertexBufferViews.at("JOINTS_0").strideInBytes),
                    static_cast<UINT>(primitive.vertexBufferViews.at("WEIGHTS_0").strideInBytes),
                };
                UINT offsets[_countof(vertexBuffers)]{ 0 };
                immediateContext->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
                immediateContext->IASetIndexBuffer(primitive.indexBufferView.buffer.Get(),
                    primitive.indexBufferView.format, 0);

                PrimitiveConstants primitiveData{};
                primitiveData.material = primitive.material;
                primitiveData.hasTangent = primitive.vertexBufferViews.at("TANGENT").buffer != NULL;
                primitiveData.skin = node.skin;
                XMStoreFloat4x4(&primitiveData.world,
                    XMLoadFloat4x4(&node.globalTransform) * XMLoadFloat4x4(&world));
                immediateContext->UpdateSubresource(primitiveCbuffer.Get(), 0, 0, &primitiveData, 0, 0);
                immediateContext->VSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());
                immediateContext->PSSetConstantBuffers(0, 1, primitiveCbuffer.GetAddressOf());

                immediateContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView.Count()), 0, 0);
            }
        }
        for (std::vector<int>::value_type childIndex : node.children)
        {
            Traverse(childIndex);
        }
    } };
    for (std::vector<int>::value_type nodeIndex : scenes.at(0).nodes)
    {
        Traverse(nodeIndex);
    }

}

void GltfModel::FetchNodes(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Node>::const_reference gltfNode : gltfModel.nodes)   
    {
        Node& node{ nodes.emplace_back() };
        node.name = gltfNode.name;
        node.skin = gltfNode.skin;
        node.mesh = gltfNode.mesh;
        node.children = gltfNode.children;

        if (!gltfNode.matrix.empty())
        {
            DirectX::XMFLOAT4X4 matrix;
            for (size_t row = 0; row < 4; row++)
            {
                for (size_t column = 0; column < 4; column++)
                {
                    matrix(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
                }
            }

            DirectX::XMVECTOR S, T, R;
            bool succeed = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
            _ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

            DirectX::XMStoreFloat3(&node.scale, S);
            DirectX::XMStoreFloat4(&node.rotation, R);
            DirectX::XMStoreFloat3(&node.translation, T);
        }
        else
        {
            if (gltfNode.scale.size() > 0)
            {
                node.scale.x = static_cast<float>(gltfNode.scale.at(0));
                node.scale.y = static_cast<float>(gltfNode.scale.at(1));
                node.scale.z = static_cast<float>(gltfNode.scale.at(2));
            }
            if (gltfNode.translation.size() > 0)
            {
                node.translation.x = static_cast<float>(gltfNode.translation.at(0));
                node.translation.y = static_cast<float>(gltfNode.translation.at(1));
                node.translation.z = static_cast<float>(gltfNode.translation.at(2));
            }
            if (gltfNode.rotation.size() > 0)
            {
                node.rotation.x = static_cast<float>(gltfNode.rotation.at(0));
                node.rotation.y = static_cast<float>(gltfNode.rotation.at(1));
                node.rotation.z = static_cast<float>(gltfNode.rotation.at(2));
                node.rotation.w = static_cast<float>(gltfNode.rotation.at(3));
            }
        }
    }
    CumulateTransforms(nodes);
}
