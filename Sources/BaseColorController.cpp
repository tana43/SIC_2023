#include "BaseColorController.h"

void BaseColorController::Initialize()
{
	auto& graphics{ Regal::Graphics::Graphics::Instance() };
	BGParticles = std::make_unique<Regal::Graphics::Particles>(graphics.GetDevice(), 2000);
	BGParticles->color = DirectX::XMFLOAT4(0, 0.3f, 1.0f, 1);
	BGParticles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(), 0);
}

void BaseColorController::Update(float elapsedTime)
{
	BGParticles->Integrate(Regal::Graphics::Graphics::Instance().GetDeviceContext(), elapsedTime);

	if (isBGColorChange)
	{
		auto& scale{ BGParticles->particleData.particleSize };
		if (changeTimer > 0)
		{
			scale -= elapsedTime * 0.08f;

			if (scale < 0)
			{
				scale = 0;
				BGParticles->color = particleColor;
				BGParticles->Initialize(Regal::Graphics::Graphics::Instance().GetDeviceContext(),
					elapsedTime);
			}

			changeTimer -= elapsedTime;
		}
		else
		{
			scale += elapsedTime * 0.08f;

			if (scale > 0.08f)
			{
				scale = 0.08f;
				isBGColorChange = false;
			}
		}
	}
}

void BaseColorController::Render()
{
	auto& graphics{ Regal::Graphics::Graphics::Instance() };
	graphics.SetStates(Regal::Graphics::ZT_ON_ZW_ON, Regal::Graphics::CULL_NONE, Regal::Graphics::ALPHA);
	graphics.GetDeviceContext()->GSSetConstantBuffers(1, 1, graphics.GetShader()->GetSceneConstanceBuffer().GetAddressOf());
	BGParticles->Render(graphics.GetDeviceContext());
}

void BaseColorController::ChangeColorBGParticle(DirectX::XMFLOAT4 color)
{
	changeTimer = changeMaxTime;
	isBGColorChange = true;
	particleColor = color;
}

DirectX::XMFLOAT4 BaseColorController::GetRundomBrightColor()
{
	DirectX::XMFLOAT4 color{};

	//base color
	float randColor = static_cast<float>(rand() % 256) / 255.0f;
	switch (rand() % 3)
	{
	case 0://R
		color.x = 1;
		switch (rand() % 2)
		{
		case 0://G
			color.y = randColor;
			break;
		case 1://B
			color.z = randColor;
			break;
		}
		break;
	case 1://G
		color.y = 1;
		switch (rand() % 2)
		{
		case 0://R
			color.x = randColor;
			break;
		case 1://B
			color.z = randColor;
			break;
		}
		break;
	case 2://B
		color.z = 1;
		switch (rand() % 2)
		{
		case 0://R
			color.x = randColor;
			break;
		case 1://G
			color.y = randColor;
			break;
		}
		break;
	}
	color.w = 1;

	return color;
}
