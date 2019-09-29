#include "stdafx.h"

//
// Model class
//

ba::Model::Model() :
	effect_type_(kBasic),
	b_reflect_(false)
{
}

void ba::Model::Draw(ID3D11DeviceContext* dc, const XMMATRIX& world)
{
	switch (effect_type_)
	{
	case kBasic:
		DrawWithBasicEffect(dc, world);
		break;

	default:
		break;
	}
}

void ba::Model::DrawWithBasicEffect(ID3D11DeviceContext* dc, const XMMATRIX& world)
{
	ID3DX11EffectTechnique* tech = nullptr;

	// Use these codes after implementing FBXLoader perfectly.
	/*if (b_reflect_)
		effects::kBasicEffect.tech(ba::BasicEffect::kLight3TexAlphaClipFogReflect);
	else
		effects::kBasicEffect.tech(ba::BasicEffect::kLight3TexAlphaClipFog);*/

	// Delete these line later.
	//
	tech = effects::kBasicEffect.tech(BasicEffect::kLight3Fog);

	Material material;
	material.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	material.diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	material.reflection = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	//__

	// Update effect variables.
	XMMATRIX world_inv_trans = mathhelper::InverseTranspose(world);
	effects::kBasicEffect.SetWorld(world);
	effects::kBasicEffect.SetWorldInvTrans(world_inv_trans);
	effects::kBasicEffect.SetTexMapping(XMMatrixIdentity());
	effects::kBasicEffect.SetMaterial(material);
	//effects::kBasicEffect.SetDiffuseMap();

	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);
	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, dc);
		mesh_.Draw(dc);
	}
}


//
// ModelInstance class
//

ba::ModelInstance::ModelInstance() :
	model_(nullptr)
{
}

void ba::ModelInstance::Draw(ID3D11DeviceContext* dc)
{
	model_->Draw(dc, XMLoadFloat4x4(&world_));
}
