#include "stdafx.h"

ID3D11RasterizerState* ba::renderstates::rasterizer::kExample = nullptr;

bool ba::renderstates::rasterizer::Init(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC rasterizer_desc{};
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = false;
	rasterizer_desc.DepthClipEnable = false;

	HRESULT res = device->CreateRasterizerState(&rasterizer_desc, &kExample);
	if (FAILED(res))
		return false;

	return true;
}

void ba::renderstates::rasterizer::Release()
{
	ReleaseCOM(kExample);
}

bool ba::renderstates::depthstencil::Init(ID3D11Device* device)
{
	// Do nothing now.
	return true;
}

void ba::renderstates::depthstencil::Release()
{
	// Do nothing now.
}

bool ba::renderstates::blend::Init(ID3D11Device* device)
{
	// Do nothing now.
	return true;
}

void ba::renderstates::blend::Release()
{
	// Do nothing now.
}

bool ba::renderstates::InitAll(ID3D11Device* device)
{
	if (!rasterizer::Init(device)) { ReleaseAll(); return false; }
	if (!depthstencil::Init(device)) { ReleaseAll(); return false; }
	if (!blend::Init(device)) { ReleaseAll(); return false; }

	return true;
}

void ba::renderstates::ReleaseAll()
{
	rasterizer::Release();
	depthstencil::Release();
	blend::Release();
}
