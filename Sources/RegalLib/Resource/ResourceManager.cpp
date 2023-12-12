#include "ResourceManager.h"

const std::shared_ptr<Regal::Resource::SkinnedMesh> ResourceManager::LoadModelResource(
    ID3D11Device* device, 
    const char* const fbxFilename, 
    const bool triangulate, 
    const float samplingRate)
{
    //���f������
    const ModelMap::iterator it = models.find(fbxFilename);
    if (it != models.end())
    {
        //�����N(����)���؂�Ă��Ȃ����m�F
        if (it->second.expired() == false)
        {
            //�����������f���Ɠ������f�������������炻���Ԃ�
            return it->second.lock();
        }
    }

    //������Ȃ������V�K���f�����\�[�X���쐬�E�ǂݍ���
    const std::shared_ptr<Regal::Resource::SkinnedMesh> model = {
        std::make_shared<Regal::Resource::SkinnedMesh>(
            device,
            fbxFilename,
            triangulate,
            samplingRate)
    };

    //�}�b�v�ɓo�^
    models[fbxFilename] = model;

    return model;
}
