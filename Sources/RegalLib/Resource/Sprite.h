#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <wrl.h>
#include <sstream>

namespace Regal::Resource
{
    class Sprite
    {
        static int num;//������ imgui�̖��O���̖h�~

    public:
        Sprite(ID3D11Device* device, const wchar_t* filename,
            std::string name = "Sprite");
        ~Sprite();

    public:
        struct SpriteTransform
        {
        private:
            DirectX::XMFLOAT2 pos = {};
            DirectX::XMFLOAT2 scale = { 1,1 };
            float angle = 0.0f;
            DirectX::XMFLOAT2 texPos = {};
            DirectX::XMFLOAT2 texSize = { 100,100 };

        public:
            void DrawDebug();

            void SetPosition(const DirectX::XMFLOAT2 p) { pos = p; }
            void SetPosition(const float x, const float y) { pos.x = x; pos.y = y; }
            void SetPositionX(const float posX) { pos.x = posX; }
            void SetPositionY(const float posY) { pos.y = posY; }
            // �����̈ʒu���摜�̒��S�Ƃ��āA�摜�̈ʒu��ݒ肷��
            void SetSpriteCenterPos(DirectX::XMFLOAT2 p)
            {
                p.x -= scale.x / 2;
                p.y -= scale.y / 2;
                pos = p;
            }
            // �������̈ʒu�𒆐S�ʒu�Ƃ��āA�摜�̈ʒu��ݒ肷��
            void SetSpriteAtAnyCenterPos(DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 center)
            {
                p.x -= center.x;
                p.y -= center.y;
                pos = p;
            }
            void SetScale(const DirectX::XMFLOAT2 s) { scale = s; }
            void SetScaleX(const float x) { scale.x = x; }
            void SetScaleY(const float y) { scale.y = y; }
            void SetScale(const float s)
            {
                scale.x = s;
                scale.y = s;
            }

            void SetAngle(const float a) { angle = a; }
            void SetTexPos(const DirectX::XMFLOAT2 texP) { texPos = texP; }
            void SetTexPosX(const float x) { texPos.x = x; }
            void SetTexPosY(const float y) { texPos.y = y; }
            void SetTexSize(const DirectX::XMFLOAT2 texS) { texSize = texS; }
            void SetTexSizeX(const float x) { texSize.x = x; }
            void SetTexSizeY(const float y) { texSize.y = y; }

            void AddPosX(const float posX) { pos.x += posX; }
            void AddPosY(const float posY) { pos.y += posY; }

            DirectX::XMFLOAT2 GetPos() { return pos; }
            float GetPosX() { return pos.x; }
            float GetPosY() { return pos.y; }
            DirectX::XMFLOAT2 GetSize() { return scale; }
            float GetScaleX() { return scale.x; }
            float GetScaleY() { return scale.y; }

            float GetAngle() { return angle; }
            DirectX::XMFLOAT2 GetTexPos() { return texPos; }
            float GetTexPosX() { return texPos.x; }
            float GetTexPosY() { return texPos.y; }
            DirectX::XMFLOAT2 GetTexSize() { return texSize; }
            float GetTexSizeX() { return texSize.x; }
            float GetTexSizeY() { return texSize.y; }
        };
    public:

        void DrawDebug();

        void Render();
        void Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size);
        void Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color);
        void Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, float angle/*degree*/);
        void Render(ID3D11DeviceContext* deviceContext, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, float angle/*degree*/, DirectX::XMFLOAT2 texPos, DirectX::XMFLOAT2 texSize); // �{��

        //����alpha�̒l�փt�F�[�h�C���E�A�E�g���� �t�F�[�h��������True��Ԃ�
        bool FadeIn(float alpha,float changeSpeed);
        bool FadeOut(float alpha, float changeSpeed);

        //���`��Ԃ̊֐����g���Ă��邪���ۂ̒l�̕ω��͂������Ⴄ
        bool FadeColor(DirectX::XMFLOAT4 newColor, float Timer,float maxTime);

        //���ǂ���
       /* void Textout(ID3D11DeviceContext* immediateContext,
            std::string s,
            float x, float y, float w, float h,
            float r, float g, float b, float a
        );*/

        /// <summary>
    /// �A�j���[�V�����X�V�֐�
    /// </summary>
    /// <param name="elapsedTime">�t���[���o�ߎ���</param>
    /// <param name="frameSpeed">�A�j���[�V�������鑬��</param>
    /// <param name="totalAnimationFrame">�A�j���[�V�����t���[����</param>
    /// <param name="animationVertical">�c�A�j���[�V�����̏ꍇtrue</param>
        void PlayAnimation(
            const float elapsedTime,
            const float frameSpeed,
            const float totalAnimationFrame,
            const bool animationVertical
        );

        //���[���h���W����X�N���[�����W�ɕύX��`��
        static DirectX::XMFLOAT2 ConvertToScreenPos(const DirectX::XMFLOAT3 worldPos);

    public:


        void SetColor(const DirectX::XMFLOAT4 c) { color = c; }
        void SetColor(const float r, const float g, const float b, const float a) { color = DirectX::XMFLOAT4(r, g, b, a); }
        void SetColorR(const float r) { color.x = r; }
        void SetColorG(const float g) { color.y = g; }
        void SetColorB(const float b) { color.z = b; }
        void SetColorA(const float a) { color.w = a; }
        void SetAlpha(const float a) { color.w = a; }

        SpriteTransform& GetSpriteTransform() { return spriteTransform; }
        DirectX::XMFLOAT4 GetColor() { return color; }
        float GetColorR() { return color.x; }
        float GetColorG() { return color.y; }
        float GetColorB() { return color.z; }
        float GetColorA() { return color.w; }
        float GetAlpha() { return color.w; }

        void SetVisibility(const bool newVisibility) { visibility = newVisibility; }
        const bool GetVisibility() { return visibility; }

    public:
        //�V�F�[�_�[���\�[�X�r���[�擾
        const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView; }

        //���_�o�b�t�@�擾
        const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer; }

        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT4 color;
            DirectX::XMFLOAT2 texcoord;
        };

    private:
        DirectX::XMFLOAT4 color = { 1,1,1,1 };

        Microsoft::WRL::ComPtr<ID3D11VertexShader>  vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   inputLayout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        vertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        D3D11_TEXTURE2D_DESC texture2dDesc;

        SpriteTransform spriteTransform;

        // Animation
        float animationTime = 0.0f;
        float animationFrame = 0.0f;

        std::string name;
        int myNum;

        bool visibility = true;//����
    };
};
