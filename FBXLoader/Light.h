#pragma once

namespace ba
{
	struct DirectionalLight
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;

		DirectX::XMFLOAT3 direction;
		float pad;

		DirectionalLight();
	};

	struct PointLight
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;

		DirectX::XMFLOAT3 pos;
		float range;

		DirectX::XMFLOAT3 attenuation;
		float pad;

		PointLight();
	};

	struct SpotLight
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;

		DirectX::XMFLOAT3 pos;
		float range;

		DirectX::XMFLOAT3 direction;
		float spot;

		DirectX::XMFLOAT3 attenuation;
		float pad;

		SpotLight();
	};

	struct Material
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;
		DirectX::XMFLOAT4 reflection;

		Material();
	};
}
