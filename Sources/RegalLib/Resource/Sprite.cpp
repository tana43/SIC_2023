#include "Sprite.h"
#include "Texture.h"
#include "../Other/Misc.h"
#include "../Graphics/Graphics.h"
#include <WICTextureLoader.h>
#include "../Game/Camera.h"
#include <algorithm>

#include "../../Easing.h"

#ifdef  USE_IMGUI
#include "../../../External/imgui/imgui.h"
#endif //  USE_IMGUI


namespace Regal::Resource
{
    int Sprite::num{};

    Sprite::Sprite(ID3D11Device* device, const wchar_t* filename, std::string name) : name(name), myNum(num++)
    {

        Vertex vertices[]
        {
            {{-1.0,+1.0,0}, {1,1,1,1},{0,0}},
            {{+1.0,+1.0,0}, {1,1,1,1},{1,0}},
            {{-1.0,-1.0,0}, {1,1,1,1},{0,1}},
            {{+1.0,-1.0,0}, {1,1,1,1},{1,1}},
        };

        HRESULT hr{ S_OK };

        //頂点バッファオブジェクト生成
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = sizeof(vertices);
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA subresourceData{};
        subresourceData.pSysMem = vertices;
        subresourceData.SysMemPitch = 0;
        subresourceData.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //頂点シェーダーオブジェクト生成
        const char* csoName{ "./Resources/Shader/SpriteVS.cso" };

        FILE* fp{};
        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO file not found");

        fseek(fp, 0, SEEK_END);
        long csoSize{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]>csoData{ std::make_unique<unsigned char[]>(csoSize) };
        fread(csoData.get(), csoSize, 1, fp);
        fclose(fp);

        hr = device->CreateVertexShader(csoData.get(), csoSize, nullptr, vertexShader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //入力レイアウトオブジェクト生成
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
        {
            { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,   0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            { "COLOR"   ,0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
            { "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,      0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        };
        hr = device->CreateInputLayout(inputElementDesc, _countof(inputElementDesc),
            csoData.get(), csoSize, inputLayout.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //ピクセルシェーダーオブジェクト生成
        csoName = "./Resources/Shader/SpritePS.cso";

        fopen_s(&fp, csoName, "rb");
        _ASSERT_EXPR_A(fp, "CSO file not found");

        fseek(fp, 0, SEEK_END);
        csoSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        csoData = std::make_unique<unsigned char[]>(csoSize);
        fread(csoData.get(), csoSize, 1, fp);
        fclose(fp);

        hr = device->CreatePixelShader(csoData.get(), csoSize, nullptr, pixelShader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        hr = LoadTextureFromFile(device, filename, shaderResourceView.GetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        // 画像サイズを設定
        spriteTransform.SetTexSize(DirectX::XMFLOAT2(texture2dDesc.Width, texture2dDesc.Height));
    }

    Sprite::~Sprite()
    {
    }

    void Sprite::Render()
    {
        using namespace Regal;
        Graphics::Graphics& graphics = Graphics::Graphics::Instance();
        Shader* shader = graphics.GetShader();

        //画面サイズを考慮した補正
        DirectX::XMFLOAT2 scareenSize{graphics.GetScreenWidth(), graphics.GetScreenHeight()};
        DirectX::XMFLOAT2 size{
            spriteTransform.GetTexSizeX()* spriteTransform.GetScaleX()* (scareenSize.x / SCREEN_WIDTH),
                spriteTransform.GetTexSizeY()* spriteTransform.GetScaleY()* (scareenSize.y / SCREEN_HEIGHT)};


        // 描画
        Render(graphics.GetDeviceContext(), spriteTransform.GetPos(), size, color,
            spriteTransform.GetAngle(), spriteTransform.GetTexPos(), spriteTransform.GetTexSize());
    }

    void Sprite::Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size)
    {
        Render(deviceContext, pos, size, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f),
            DirectX::XMFLOAT2(static_cast<float>(texture2dDesc.Width), static_cast<float>(texture2dDesc.Height)));
    }

    void Sprite::Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color)
    {
        Render(deviceContext, pos, size, color, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f),
            DirectX::XMFLOAT2(static_cast<float>(texture2dDesc.Width), static_cast<float>(texture2dDesc.Height)));
    }

    void Sprite::Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, float angle)
    {
        Render(deviceContext, pos, size, color, angle, DirectX::XMFLOAT2(0.0f, 0.0f),
            DirectX::XMFLOAT2(static_cast<float>(texture2dDesc.Width), static_cast<float>(texture2dDesc.Height)));
    }

    void Sprite::Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, float angle, DirectX::XMFLOAT2 texPos, DirectX::XMFLOAT2 texSize)
    {
        if (!visibility)return;

        HRESULT hr{ S_OK };

        // スクリーン（ビューポート）のサイズを取得する
        D3D11_VIEWPORT viewport{};
        UINT numViewports{ 1 };
        deviceContext->RSGetViewports(&numViewports, &viewport);

        // 短形の角頂点の位置（スクリーン座標系）を計算する
        // (x0, y0) *----* (x1, y1)
        //          |   /|
        //          |  / |
        //          | /  |
        //          |/   |
        // (x2, y2) *----* (x3, y3)

        // left-top
        float x0{ pos.x };
        float y0{ pos.y };
        // right-top
        float x1{ pos.x + size.x };
        float y1{ pos.y };
        // left-bottom
        float x2{ pos.x };
        float y2{ pos.y + size.y };
        // right-bottom
        float x3{ pos.x + size.x };
        float y3{ pos.y + size.y };

        // 矩形回転
        auto rotate = [](float& x, float& y, float cx, float cy, float angle)
        {
            x -= cx;
            y -= cy;

            float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
            float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
            float tx{ x }, ty{ y };
            x = cos * tx + -sin * ty;
            y = sin * tx + cos * ty;

            x += cx;
            y += cy;
        };
        // 回転の中心を短形の中心点にした場合
        float cx = pos.x + size.x * 0.5f;
        float cy = pos.y + size.y * 0.5f;
        rotate(x0, y0, cx, cy, angle);
        rotate(x1, y1, cx, cy, angle);
        rotate(x2, y2, cx, cy, angle);
        rotate(x3, y3, cx, cy, angle);

        // スクリーン座標系からNDCへの座標変換をおこなう
        x0 = 2.0f * x0 / viewport.Width - 1.0f;
        y0 = 1.0f - 2.0f * y0 / viewport.Height;
        x1 = 2.0f * x1 / viewport.Width - 1.0f;
        y1 = 1.0f - 2.0f * y1 / viewport.Height;
        x2 = 2.0f * x2 / viewport.Width - 1.0f;
        y2 = 1.0f - 2.0f * y2 / viewport.Height;
        x3 = 2.0f * x3 / viewport.Width - 1.0f;
        y3 = 1.0f - 2.0f * y3 / viewport.Height;

        // 計算結果で頂点バッファオブジェクトを更新する
        D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
        hr = deviceContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        Vertex* vertices{ reinterpret_cast<Vertex*>(mapped_subresource.pData) };
        if (vertices != nullptr)
        {
            vertices[0].position = { x0, y0, 0 };
            vertices[1].position = { x1, y1, 0 };
            vertices[2].position = { x2, y2, 0 };
            vertices[3].position = { x3, y3, 0 };
            vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = color;

            vertices[0].texcoord = { texPos.x / texture2dDesc.Width, texPos.y / texture2dDesc.Height };
            vertices[1].texcoord = { (texPos.x + texSize.x) / texture2dDesc.Width, texPos.y / texture2dDesc.Height };
            vertices[2].texcoord = { texPos.x / texture2dDesc.Width, (texPos.y + texSize.y) / texture2dDesc.Height };
            vertices[3].texcoord = { (texPos.x + texSize.x) / texture2dDesc.Width, (texPos.y + texSize.y) / texture2dDesc.Height };
        }
        deviceContext->Unmap(vertexBuffer.Get(), 0);

        UINT stride{ sizeof(Vertex) };
        UINT offset{ 0 };

        deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        deviceContext->IASetInputLayout(inputLayout.Get());

        deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
        deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

        deviceContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

        deviceContext->Draw(4, 0);
        //---これより下に何かいても意味ない---//
    }

    bool Sprite::FadeIn(float alpha, float changeSpeed)
    {
        if (color.w + changeSpeed >= alpha)
        {
            color.w = alpha;
            return true;
        }

        color.w += changeSpeed;
        return false;
    }

    bool Sprite::FadeOut(float alpha, float changeSpeed)
    {
        if (color.w - changeSpeed <= alpha)
        {
            color.w = alpha;
            return true;
        }

        color.w -= changeSpeed;
        return false;
    }

    bool Sprite::FadeColor(DirectX::XMFLOAT4 newColor, float Timer, float maxTime)
    {
        std::clamp(Timer,0.0f,maxTime);
        float alpha = Timer / maxTime;
        DirectX::XMVECTOR result = DirectX::XMVectorLerp(DirectX::XMLoadFloat4(&color), DirectX::XMLoadFloat4(&newColor), alpha);
        DirectX::XMStoreFloat4(&color, result);

        if (Timer == maxTime)return true;

        return false;
    }

    
    void Sprite::PlayAnimation(const float elapsedTime, const float frameSpeed, const float totalAnimationFrame, const bool animationVertical)
    {
        animationTime += frameSpeed * elapsedTime;

        const int frame = static_cast<int>(animationTime); // 小数点切り捨て
        animationFrame = static_cast<float>(frame);

        if (animationFrame > totalAnimationFrame)
        {
            animationFrame = 0.0f;
            animationTime = 0.0f;
        }

        DirectX::XMFLOAT2 texPos = spriteTransform.GetTexPos();
        const DirectX::XMFLOAT2 texSize = spriteTransform.GetTexSize();

        if (animationVertical) texPos.y = texSize.y * animationFrame;
        else                   texPos.x = texSize.x * animationFrame;

        spriteTransform.SetTexPos(texPos);
    }

    DirectX::XMFLOAT2 Sprite::ConvertToScreenPos(const DirectX::XMFLOAT3 worldPos)
    {
        auto* dc = Regal::Graphics::Graphics::Instance().GetDeviceContext();
        //ビューポート
        D3D11_VIEWPORT viewport;
        UINT numViewports = 1;
        dc->RSGetViewports(&numViewports, &viewport);

        DirectX::XMMATRIX View = Regal::Game::Camera::Instance().CalcViewMatrix();
        DirectX::XMMATRIX Projection = Regal::Game::Camera::Instance().CalcProjectionMatrix();
        DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

        DirectX::XMVECTOR WorldPosition = DirectX::XMLoadFloat3(&worldPos);

        DirectX::XMVECTOR ScreenPosition = DirectX::XMVector3Project(
            WorldPosition,
            viewport.TopLeftX, viewport.TopLeftY,
            viewport.Width, viewport.Height,
            viewport.MinDepth, viewport.MaxDepth,
            Projection, View, World
        );

        DirectX::XMFLOAT2 screenPosition;
        DirectX::XMStoreFloat2(&screenPosition, ScreenPosition);

        const float screenPositionZ = DirectX::XMVectorGetZ(ScreenPosition);

        return screenPosition;
    }

    /*void Sprite::Textout(ID3D11DeviceContext* immediateContext, std::string s, float x, float y, float w, float h, float r, float g, float b, float a)
    {
        float sw = static_cast<float>(texture2dDesc.Width / 16);
        float sh = static_cast<float>(texture2dDesc.Height / 16);
        float carriage = 0;
        for (const char c : s)
        {
            Render(immediateContext, x + carriage, y, w, h, r, g, b, a, 0,
                sw * (c & 0x0F), sh * (c >> 4), sw, sh);
            carriage += w;
        }
    }*/

    void Sprite::DrawDebug()
    {
#ifdef USE_IMGUI

        std::string n = name + "_" + std::to_string(myNum);
        if (ImGui::BeginMenu(n.c_str()))
        {
            ImGui::Begin(n.c_str());
            ImGui::ColorEdit4("color", &color.x, ImGuiColorEditFlags_PickerHueWheel);

            spriteTransform.DrawDebug();

            ImGui::End();
            ImGui::EndMenu();
        }


#endif
    }
    void Sprite::SpriteTransform::DrawDebug()
    {
        if (ImGui::TreeNode("spriteTransform"))
        {
            ImGui::DragFloat2("pos", &pos.x);
            ImGui::DragFloat2("scale", &scale.x,0.05f);

            ImGui::DragFloat("angle", &angle);
            ImGui::DragFloat2("texPos", &texPos.x);
            ImGui::DragFloat2("texSize", &texSize.x);
            ImGui::TreePop();
        }
    }
}