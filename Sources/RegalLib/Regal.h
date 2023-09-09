#pragma once

//�I�t�X�N���[�������_�����O����
//#define DISABLE_OFFSCREENRENDERING 

//�f�����[�h�ɐ؂�ւ�
//#define DEMO_MODE

#include "Other/Misc.h"

#include <d3d11.h>
#include <wrl.h>

//std::clamp�Ƃ��g����悤�ɂ�����
#include <algorithm>

#include "Resource/Sprite.h"
#include "Resource/SpriteBatch.h"
#include "Resource/GeometricPrimitive.h"
#include "Resource/StaticMesh.h"
#include "Resource/SkinnedMesh.h"
#include "Resource/GltfModel.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"

#include "Graphics/Graphics.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/FullscreenQuad.h"
#include "Graphics/Bloom.h"
#include "Graphics/SkyBox.h"
#include "Graphics/Particles.h"

#include "Model/StaticModel.h"

#include "Game/Transform.h"
#include "Game/Camera.h"
#include "Game/GameObject.h"

#include "Helper/MathHelper.h"

#include "Scene/BaseScene.h"
#include "Scene/SceneManager.h"

#include "Input/Input.h"

#include "Demo/DemoScene.h"
