#include "stdafx.h"

ba::Renderer::Renderer() :
	dc_(nullptr)
{
}

ba::Renderer::~Renderer()
{
}

bool ba::Renderer::Init(ID3D11Device* device, ID3D11DeviceContext* dc)
{
	dc_ = dc;

	return true;
}

void ba::Renderer::Release()
{
}

void ba::Renderer::RenderNormaly(const std::vector<ModelInstance>& model_instances)
{
	dc_->IASetInputLayout(inputvertex::PosNormalTex::kInputLayout);

	ID3DX11EffectTechnique* tech = nullptr;

	// Add this line after implementing 'FBXLoader' completely(loading texture part).
	//tech = effects::kBasicEffect.tech(BasicEffect::kLight3TexAlphaClipFog);

	// Delete this line later.
	tech = effects::kBasicEffect.tech(BasicEffect::kLight3Fog);

	XMMATRIX world, world_inv_trans;

	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);
	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		for (UINT i = 0; i < model_instances.size(); ++i)
		{
			// Update effect variables.
			world = XMLoadFloat4x4(&model_instances[i].world_);
			world_inv_trans = mathhelper::InverseTranspose(world);
			effects::kBasicEffect.SetWorld(world);
			effects::kBasicEffect.SetWorldInvTrans(world_inv_trans);
			effects::kBasicEffect.SetTexMapping(XMMatrixIdentity());
			effects::kBasicEffect.SetMaterial(model_instances[i].model_->mesh_.material());
			//effects::kBasicEffect.SetDiffuseMap();

			tech->GetPassByIndex(p)->Apply(0, dc_);
			model_instances[i].model_->mesh_.Draw(dc_);
		}
	}
}

void ba::Renderer::RenderShadowMap(const std::vector<ModelInstance>& model_instances, ShadowMap& shadow_map)
{
	ID3D11RenderTargetView* rtvs_[1] = { nullptr };
	dc_->OMSetRenderTargets(1, rtvs_, shadow_map.dsv());
	dc_->ClearDepthStencilView(shadow_map.dsv(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	dc_->RSSetViewports(1, &shadow_map.viewport());

	dc_->IASetInputLayout(inputvertex::PosNormalTex::kInputLayout);

	ID3DX11EffectTechnique* tech = nullptr;

	// Use these codes after implementing FBXLoader perfectly.
	//tech = effects::kShadowMapEffect.tech(ShadowMapEffect::kBuildShadowMap);

	// Delete this line later.
	tech = effects::kShadowMapEffect.tech(ShadowMapEffect::kBuildShadowMap);

	XMMATRIX world, world_inv_trans;

	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);
	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		for (UINT i = 0; i < model_instances.size(); ++i)
		{
			world = XMLoadFloat4x4(&model_instances[i].world_);
			world_inv_trans = mathhelper::InverseTranspose(world);
			effects::kShadowMapEffect.SetWorld(world);
			effects::kShadowMapEffect.SetWorldInvTrans(world_inv_trans);
			effects::kShadowMapEffect.SetTexMapping(XMMatrixIdentity());
			//effects::kShadowMapEffect.SetDiffuseMap();

			tech->GetPassByIndex(p)->Apply(0, dc_);
			model_instances[i].model_->mesh_.Draw(dc_);
		}
	}
	dc_->RSSetState(nullptr);
}

void ba::Renderer::RenderNormalDepthMap(const std::vector<ModelInstance>& model_instances, SSAOMap& ssao_map)
{
	ID3D11RenderTargetView* rtvs_[1] = { ssao_map.normal_depth_map_rtv() };
	dc_->OMSetRenderTargets(1, rtvs_, ssao_map.dsv());

	float clear_color[4] = { 0.0f, 0.0f, -1.0f, 1e5f };
	dc_->ClearRenderTargetView(rtvs_[0], clear_color);
	dc_->ClearDepthStencilView(ssao_map.dsv(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	dc_->IASetInputLayout(inputvertex::PosNormalTex::kInputLayout);

	ID3DX11EffectTechnique* tech = nullptr;

	// Use these codes after implementing FBXLoader perfectly.
	//tech = effects::kNormalDepthMapEffect.tech(NormalDepthMapEffect::kNormalDepthMapAlphaClip);

	// Delete this line later.
	tech = effects::kNormalDepthMapEffect.tech(NormalDepthMapEffect::kNormalDepthMap);

	XMMATRIX world, world_inv_trans;

	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);
	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		for (UINT i = 0; i < model_instances.size(); ++i)
		{
			world = XMLoadFloat4x4(&model_instances[i].world_);
			world_inv_trans = mathhelper::InverseTranspose(world);
			effects::kNormalDepthMapEffect.SetWorld(world);
			effects::kNormalDepthMapEffect.SetWorldInvTrans(world_inv_trans);
			effects::kNormalDepthMapEffect.SetTexMapping(XMMatrixIdentity());
			//effects::kNormalDepthMapEffect.SetDiffuseMap();

			tech->GetPassByIndex(p)->Apply(0, dc_);
			model_instances[i].model_->mesh_.Draw(dc_);
		}
	}
}
