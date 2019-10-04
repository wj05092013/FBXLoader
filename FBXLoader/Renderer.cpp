#include "stdafx.h"

ba::Renderer::Renderer() :
	dc_(nullptr),
	rendering_component_{}
{
}

ba::Renderer::~Renderer()
{
}

bool ba::Renderer::Init(ID3D11Device* device, ID3D11DeviceContext* dc)
{
	dc_ = dc;

	if (!effects::InitAll(device)) { Release(); return false; }
	if(!renderstates::InitAll(device)) { Release(); return false; }

	return true;
}

void ba::Renderer::Release()
{
	effects::ReleaseAll();
	renderstates::ReleaseAll();
}

void ba::Renderer::RenderScene(const std::vector<ModelInstance>& model_instances, const EffectVariableBundlePerFrame& bundle)
{
	dc_->RSSetState(renderstates::rasterizer::kExample);

	ID3D11RenderTargetView* rtvs_[1] = { rendering_component_.rtv };
	dc_->OMSetRenderTargets(1, rtvs_, rendering_component_.dsv);
	dc_->RSSetViewports(1, rendering_component_.viewport);

	dc_->ClearRenderTargetView(rendering_component_.rtv, reinterpret_cast<const float*>(&color::kMagenta));
	dc_->ClearDepthStencilView(rendering_component_.dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);

	dc_->IASetInputLayout(inputvertex::PosNormalTex::kInputLayout);

	ID3DX11EffectTechnique* tech = nullptr;

	// Add this line after implementing 'FBXLoader' completely(loading texture part).
	//tech = effects::kBasicEffect.tech(BasicEffect::kLight3TexAlphaClipFog);

	// Delete this line later.
	tech = effects::kBasicEffect.tech(BasicEffect::kLight3Fog);

	// Set effect variables.
	effects::kBasicEffect.SetEyePos(rendering_component_.cam->position_w_xf());
	effects::kBasicEffect.SetView(rendering_component_.cam->view());
	effects::kBasicEffect.SetShadowTransform(rendering_component_.shadow_map->world_to_tex());
	effects::kBasicEffect.SetShadowMap(rendering_component_.shadow_map->srv());
	effects::kBasicEffect.SetSSAOMap(rendering_component_.ssao_map->ssao_map_srv());

	XMMATRIX world, world_inv_trans;

	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);
	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		for (UINT i = 0; i < model_instances.size(); ++i)
		{
			// Update effect variables.
			world = XMLoadFloat4x4(&model_instances[i].world_);
			world = mathhelper::kRhToLh * world;
			world_inv_trans = mathhelper::InverseTranspose(world);
			effects::kBasicEffect.SetWorld(world);
			effects::kBasicEffect.SetWorldInvTrans(world_inv_trans);
			effects::kBasicEffect.SetTexMapping(XMMatrixIdentity());
			effects::kBasicEffect.SetMaterial(model_instances[i].model_->mesh_.material());
			//effects::kBasicEffect.SetDiffuseMap();

			tech->GetPassByIndex(p)->Apply(0, dc_);
			model_instances[i].model_->mesh_.Draw(dc_);
		}

		// Unbind the SRVs in case the resources are bound as render targets.
		effects::kBasicEffect.SetShadowMap(nullptr);
		effects::kBasicEffect.SetSSAOMap(nullptr);
		tech->GetPassByIndex(p)->Apply(0, dc_);
	}
	dc_->RSSetState(nullptr);
}

void ba::Renderer::RenderShadowMap(const std::vector<ModelInstance>& model_instances, const EffectVariableBundlePerFrame& bundle)
{
	dc_->RSSetState(renderstates::rasterizer::kExample);

	ID3D11RenderTargetView* rtvs_[1] = { nullptr };
	dc_->OMSetRenderTargets(1, rtvs_, rendering_component_.shadow_map->dsv());
	dc_->ClearDepthStencilView(rendering_component_.shadow_map->dsv(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	dc_->RSSetViewports(1, &rendering_component_.shadow_map->viewport());

	dc_->IASetInputLayout(inputvertex::PosNormalTex::kInputLayout);

	ID3DX11EffectTechnique* tech = nullptr;

	// Use these codes after implementing FBXLoader perfectly.
	//tech = effects::kShadowMapEffect.tech(ShadowMapEffect::kBuildShadowMap);

	// Delete this line later.
	tech = effects::kShadowMapEffect.tech(ShadowMapEffect::kBuildShadowMap);

	// Set effect variables.
	effects::kShadowMapEffect.SetLightView(rendering_component_.shadow_map->view());
	effects::kShadowMapEffect.SetLightProj(rendering_component_.shadow_map->proj());

	XMMATRIX world, world_inv_trans;

	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);
	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		for (UINT i = 0; i < model_instances.size(); ++i)
		{
			world = XMLoadFloat4x4(&model_instances[i].world_);
			world = mathhelper::kRhToLh * world;
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

void ba::Renderer::RenderNormalDepthMap(const std::vector<ModelInstance>& model_instances, const EffectVariableBundlePerFrame& bundle)
{
	dc_->RSSetState(renderstates::rasterizer::kExample);

	ID3D11RenderTargetView* rtvs_[1] = { rendering_component_.ssao_map->normal_depth_map_rtv() };
	dc_->OMSetRenderTargets(1, rtvs_, rendering_component_.ssao_map->dsv());
	dc_->RSSetViewports(1, rendering_component_.viewport);

	float clear_color[4] = { 0.0f, 0.0f, -1.0f, 1e5f };
	dc_->ClearRenderTargetView(rtvs_[0], clear_color);
	dc_->ClearDepthStencilView(rendering_component_.ssao_map->dsv(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	dc_->IASetInputLayout(inputvertex::PosNormalTex::kInputLayout);

	ID3DX11EffectTechnique* tech = nullptr;

	// Use these codes after implementing FBXLoader perfectly.
	//tech = effects::kNormalDepthMapEffect.tech(NormalDepthMapEffect::kNormalDepthMapAlphaClip);

	// Delete this line later.
	tech = effects::kNormalDepthMapEffect.tech(NormalDepthMapEffect::kNormalDepthMap);

	// Set effect variables.
	effects::kNormalDepthMapEffect.SetView(rendering_component_.cam->view());

	XMMATRIX world, world_inv_trans;

	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);
	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		for (UINT i = 0; i < model_instances.size(); ++i)
		{
			world = XMLoadFloat4x4(&model_instances[i].world_);
			world = mathhelper::kRhToLh * world;
			world_inv_trans = mathhelper::InverseTranspose(world);
			effects::kNormalDepthMapEffect.SetWorld(world);
			effects::kNormalDepthMapEffect.SetWorldInvTrans(world_inv_trans);
			effects::kNormalDepthMapEffect.SetTexMapping(XMMatrixIdentity());
			//effects::kNormalDepthMapEffect.SetDiffuseMap();

			tech->GetPassByIndex(p)->Apply(0, dc_);
			model_instances[i].model_->mesh_.Draw(dc_);
		}
	}
	dc_->RSSetState(nullptr);
}

void ba::Renderer::SetEffectVariablesOnResize(const EffectVariableBundlePerResize& bundle)
{
	// BasicEffect
	effects::kBasicEffect.SetProj(rendering_component_.cam->proj());

	// ShadowMapEffect

	// NormalDepthMapEffect
	effects::kNormalDepthMapEffect.SetProj(rendering_component_.cam->proj());
}

void ba::Renderer::SetEffectVariablesChangeRarely(const EffectVariableBundleChangeRarely& bundle)
{
	// BasicEffect
	effects::kBasicEffect.SetDirectionalLights(bundle.directional_lights);
	effects::kBasicEffect.SetFogStart(bundle.fog_start);
	effects::kBasicEffect.SetFogRange(bundle.fog_range);
	effects::kBasicEffect.SetFogColor(bundle.fog_color);
	effects::kBasicEffect.SetShadowMapSize(bundle.shadow_map_size);
	effects::kBasicEffect.SetToTex(bundle.to_tex);

	// ShadowMapEffect

	// NormalDepthMapEffect
}

void ba::Renderer::set_rendering_component(RenderingComponent& rendering_component)
{
	rendering_component_ = rendering_component;
}
