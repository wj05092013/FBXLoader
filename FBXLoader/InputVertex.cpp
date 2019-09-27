#include "stdafx.h"

//
// PosNormalTex
//

const D3D11_INPUT_ELEMENT_DESC ba::inputvertex::PosNormalTex::kInputElemDesc[3] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

bool ba::inputvertex::PosNormalTex::Init(ID3D11Device* device)
{
	if (input_layout)
		return false;

	ID3DX11EffectTechnique* tech = effects::kBasicEffect.tech(BasicEffect::TechType::kLight1);
	D3DX11_PASS_DESC pass_desc;
	tech->GetPassByIndex(0)->GetDesc(&pass_desc);

	HRESULT res = device->CreateInputLayout(
		kInputElemDesc, 3,
		pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, &input_layout
	);
	if (FAILED(res))
		return false;
	return true;
}

void ba::inputvertex::PosNormalTex::Release()
{
	ReleaseCOM(input_layout);
}


//
// PosNormalTexTangent
//

const D3D11_INPUT_ELEMENT_DESC ba::inputvertex::PosNormalTexTangent::kInputElemDesc[4] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

bool ba::inputvertex::PosNormalTexTangent::Init(ID3D11Device* device)
{
	if (input_layout)
		return false;

	ID3DX11EffectTechnique* tech = effects::.tech();
	D3DX11_PASS_DESC pass_desc;
	tech->GetPassByIndex(0)->GetDesc(&pass_desc);

	HRESULT res = device->CreateInputLayout(
		kInputElemDesc, 6,
		pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, &input_layout
	);
	if (FAILED(res))
		return false;
	return true;
}

void ba::inputvertex::PosNormalTexTangent::Release()
{
	ReleaseCOM(input_layout);
}


//
// PosNormalTexTanSkinned
//

const D3D11_INPUT_ELEMENT_DESC ba::inputvertex::PosNormalTexTanSkinned::kInputElemDesc[6] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

bool ba::inputvertex::PosNormalTexTanSkinned::Init(ID3D11Device* device)
{
	if (input_layout)
		return false;

	ID3DX11EffectTechnique* tech = effects::kBasicEffect.tech(BasicEffect::TechType::kLight1Skinned);
	D3DX11_PASS_DESC pass_desc;
	tech->GetPassByIndex(0)->GetDesc(&pass_desc);

	HRESULT res = device->CreateInputLayout(
		kInputElemDesc, 6,
		pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, &input_layout
	);
	if (FAILED(res))
		return false;
	return true;
}

void ba::inputvertex::PosNormalTexTanSkinned::Release()
{
	ReleaseCOM(input_layout);
}


//
// Integrated management of all input layouts.
//

bool ba::inputvertex::InitAll(ID3D11Device* device)
{
	if (!kPosNormalTex.Init(device)) return false;
	if (!kPosNormalTexTanSkinned.Init(device)) { ReleaseAll(); return false; }

	return true;
}

void ba::inputvertex::ReleaseAll()
{
	kPosNormalTex.Release();
	kPosNormalTexTanSkinned.Release();
}
