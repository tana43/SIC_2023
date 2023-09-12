#include "BaseColorController.h"

DirectX::XMFLOAT4 BaseColorController::RundomBrightColor()
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
