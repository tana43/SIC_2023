#pragma once
#include <DirectXMath.h>
// •‚“®¬”ŽZp
class Mathf
{
public:
	// üŒ`•âŠ®
	static float Lerp(float a, float b, float t)
	{
		return a * (1.0f - t) + (b * t);
	}

};

inline DirectX::XMFLOAT2  operator-(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2& f2)
{
	DirectX::XMFLOAT2 ret{};
	ret.x = f1.x - f2.x;
	ret.y = f1.y - f2.y;
	return ret;
}

inline DirectX::XMFLOAT2  operator*(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2& f2)
{
	DirectX::XMFLOAT2 ret{};
	ret.x = f1.x * f2.x;
	ret.y = f1.y * f2.y;
	return ret;
}

inline DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	DirectX::XMFLOAT3 ret{};
	ret.x = f1.x + f2.x;
	ret.y = f1.y + f2.y;
	ret.z = f1.z + f2.z;
	return ret;
}

inline DirectX::XMFLOAT3  operator-(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	DirectX::XMFLOAT3 ret{};
	ret.x = f1.x - f2.x;
	ret.y = f1.y - f2.y;
	ret.z = f1.z - f2.z;
	return ret;
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	DirectX::XMFLOAT3 ret{};
	ret.x = f1.x * f2.x;
	ret.y = f1.y * f2.y;
	ret.z = f1.z * f2.z;
	return ret;
}


inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& f1, float f)
{
	DirectX::XMFLOAT3 ret{};
	ret.x = f1.x * f;
	ret.y = f1.y * f;
	ret.z = f1.z * f;

	return ret;
}

inline DirectX::XMFLOAT4 operator+(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret{};
	ret.x = f1.x + f2.x;
	ret.y = f1.y + f2.y;
	ret.z = f1.z + f2.z;
	ret.w = f1.w + f2.w;
	return ret;
}

inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& f)
{
	DirectX::XMFLOAT3 ret{};
	DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&f)));
	return ret;
}

inline DirectX::XMFLOAT4 ToRadians(const DirectX::XMFLOAT4& f)
{
	return DirectX::XMFLOAT4(
		DirectX::XMConvertToRadians(f.x),
		DirectX::XMConvertToRadians(f.y),
		DirectX::XMConvertToRadians(f.z),
		DirectX::XMConvertToRadians(f.w)
	);
}
inline DirectX::XMFLOAT4 operator-(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret{};
	ret.x = f1.x - f2.x;
	ret.y = f1.y - f2.y;
	ret.z = f1.z - f2.z;
	ret.w = f1.w - f2.w;
	return ret;
}

inline DirectX::XMFLOAT4 Max(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret{};
	ret.x = f1.x > f2.x ? f1.x : f2.x;
	ret.y = f1.y > f2.y ? f1.y : f2.y;
	ret.z = f1.z > f2.z ? f1.z : f2.z;
	ret.w = f1.w > f2.w ? f1.w : f2.w;
	return ret;
}

inline DirectX::XMFLOAT4 Min(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret{};
	ret.x = f1.x < f2.x ? f1.x : f2.x;
	ret.y = f1.y < f2.y ? f1.y : f2.y;
	ret.z = f1.z < f2.z ? f1.z : f2.z;
	ret.w = f1.w < f2.w ? f1.w : f2.w;
	return ret;
}

inline float Length(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	float ret{};
	DirectX::XMStoreFloat(
		&ret,
		DirectX::XMVector3Length(
			DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&f1),
				DirectX::XMLoadFloat3(&f2))));
	return ret;
}

inline float Length(const DirectX::XMFLOAT3& f)
{
	float ret;
	DirectX::XMStoreFloat(&ret, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&f)));
	return ret;
}

inline float LengthV3(const DirectX::XMVECTOR& v)
{
	float ret;
	DirectX::XMStoreFloat(&ret, DirectX::XMVector3Length(v));
	return ret;
}

inline DirectX::XMFLOAT4 Absolute(const DirectX::XMFLOAT4& f)
{
	DirectX::XMFLOAT4 ret;
	ret.x = fabsf(f.x);
	ret.y = fabsf(f.y);
	ret.z = fabsf(f.z);
	ret.w = fabsf(f.w);
	return ret;
}