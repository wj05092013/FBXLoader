#include "stdafx.h"
#include "FBXLoaderTester.h"

const UINT ba::FBXLoaderTester::kShadowMapSize = 2048;

const DirectX::XMVECTOR ba::FBXLoaderTester::kInitCamPos = XMVectorSet(0.0f, 1.0f, -15.0f, 1.0f);
const DirectX::XMVECTOR ba::FBXLoaderTester::kInitCamTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
const DirectX::XMVECTOR ba::FBXLoaderTester::kInitCamUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

const float ba::FBXLoaderTester::kFovY = 0.25f * XM_PI;
const float ba::FBXLoaderTester::kNearZ = 0.1f;
const float ba::FBXLoaderTester::kFarZ = 1000.0f;

const float ba::FBXLoaderTester::kForwardMovingRate = 10.0f;
const float ba::FBXLoaderTester::kRightMovingRate = 10.0f;
const float ba::FBXLoaderTester::kUpperMovingRate = 10.0f;
const float ba::FBXLoaderTester::kRotationRate = 0.15f;

ba::FBXLoaderTester::FBXLoaderTester() :
	scene_bounds_{},

	wolf_model_(nullptr),
	
	last_mouse_pos_{}
{
}

ba::FBXLoaderTester::~FBXLoaderTester()
{
}

void ba::FBXLoaderTester::Render()
{
	SetEffectVariablesPerFrame();

	renderer_.RenderShadowMap(model_instances_, shadow_map_);

	renderer_.RenderNormalDepthMap(model_instances_, ssao_map_);
	ssao_map_.BuildSSAOMap(cam_);
	ssao_map_.BlurSSAOMap(4);

	renderer_.RenderNormaly(model_instances_, rtv_, dsv_, &viewport_);

	swap_chain_->Present(0U, 0U);
}

void ba::FBXLoaderTester::UpdateDirectX()
{
	cam_.UpdateViewMatrix();
	shadow_map_.BuildShadowTransform(lights_[0], scene_bounds_);
}

bool ba::FBXLoaderTester::OnResize()
{
	if (!Application::OnResize())
		return false;

	cam_.SetLens(kFovY, aspect_ratio(), kNearZ, kFarZ);
	ssao_map_.OnResize(client_width_, client_height_, kFovY, kFarZ);

	SetEffectVariablesPerResize();

	return true;
}

bool ba::FBXLoaderTester::InitDirectX()
{
	if (!Application::InitDirectX())
		return false;

	if (!effects::InitAll(device_))
		return false;
	if (!inputvertex::InitAll(device_))
		return false;

	if (!renderer_.Init(device_, dc_))
		return false;
	if (!shadow_map_.Init(device_, kShadowMapSize, kShadowMapSize))
		return false;
	if (!ssao_map_.Init(device_, dc_, client_width_, client_height_, kFovY, kFarZ))
		return false;

	FBXLoader::GetInstance().Init();

	if (!InitModels())
		return false;

	InitSceneBounds();
	InitLights();

	SetEffectVariablesChangeRarely();

	return true;
}

void ba::FBXLoaderTester::ReleaseDirectX()
{
	renderer_.Release();
	shadow_map_.Release();
	ssao_map_.Release();
	ReleaseModels();

	FBXLoader::GetInstance().Release();

	effects::ReleaseAll();
	inputvertex::ReleaseAll();

	Application::ReleaseDirectX();
}

void ba::FBXLoaderTester::InitSceneBounds()
{
	scene_bounds_.center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scene_bounds_.radius = 100.0f;
}

void ba::FBXLoaderTester::InitLights()
{
	lights_[0].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	lights_[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lights_[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lights_[0].direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	lights_[1].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lights_[1].diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	lights_[1].specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	lights_[1].direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	lights_[2].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lights_[2].diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	lights_[2].specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lights_[2].direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
}

bool ba::FBXLoaderTester::InitModels()
{
	// Simple material.
	Material material;
	material.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	material.diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	material.specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	material.reflection = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	// Create an wolf model.
	//
	std::string file_name = "Model/Wolf.fbx";
	wolf_model_ = new Model;

	if (!FBXLoader::GetInstance().Load(file_name, *wolf_model_))
		return false;

	wolf_model_->mesh_.Init(device_);
	wolf_model_->mesh_.set_material(material);
	//__

	// Createa floor model.
	//
	floor_model_ = new Model;
	floor_model_->mesh_.vertices_.resize(6);

	floor_model_->mesh_.vertices_[0].pos = XMFLOAT3(-25.0f, 0.0f, 25.0f);
	floor_model_->mesh_.vertices_[0].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	floor_model_->mesh_.vertices_[0].uv = XMFLOAT2(0.0f, 0.0f);
	floor_model_->mesh_.vertices_[0].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

	floor_model_->mesh_.vertices_[1].pos = XMFLOAT3(25.0f, 0.0f, 25.0f);
	floor_model_->mesh_.vertices_[1].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	floor_model_->mesh_.vertices_[1].uv = XMFLOAT2(1.0f, 0.0f);
	floor_model_->mesh_.vertices_[1].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

	floor_model_->mesh_.vertices_[2].pos = XMFLOAT3(-25.0f, 0.0f, -25.0f);
	floor_model_->mesh_.vertices_[2].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	floor_model_->mesh_.vertices_[2].uv = XMFLOAT2(0.0f, 1.0f);
	floor_model_->mesh_.vertices_[2].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

	floor_model_->mesh_.vertices_[3].pos = XMFLOAT3(-25.0f, 0.0f, -25.0f);
	floor_model_->mesh_.vertices_[3].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	floor_model_->mesh_.vertices_[3].uv = XMFLOAT2(0.0f, 1.0f);
	floor_model_->mesh_.vertices_[3].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

	floor_model_->mesh_.vertices_[4].pos = XMFLOAT3(25.0f, 0.0f, 25.0f);
	floor_model_->mesh_.vertices_[4].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	floor_model_->mesh_.vertices_[4].uv = XMFLOAT2(1.0f, 0.0f);
	floor_model_->mesh_.vertices_[4].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

	floor_model_->mesh_.vertices_[5].pos = XMFLOAT3(25.0f, 0.0f, -25.0f);
	floor_model_->mesh_.vertices_[5].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	floor_model_->mesh_.vertices_[5].uv = XMFLOAT2(1.0f, 1.0f);
	floor_model_->mesh_.vertices_[5].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

	floor_model_->mesh_.Init(device_);
	floor_model_->mesh_.set_material(material);
	//__

	ModelInstance model_inst;

	model_inst.model_ = wolf_model_;
	XMStoreFloat4x4(&model_inst.world_, XMMatrixIdentity());
	model_instances_.push_back(model_inst);

	model_inst.model_ = floor_model_;
	XMStoreFloat4x4(&model_inst.world_, XMMatrixIdentity());
	model_instances_.push_back(model_inst);

	return true;
}

void ba::FBXLoaderTester::ReleaseModels()
{
	if (wolf_model_)
	{
		delete wolf_model_;
		wolf_model_ = nullptr;
	}
	if (floor_model_)
	{
		delete floor_model_;
		floor_model_ = nullptr;
	}

	model_instances_.clear();
}

void ba::FBXLoaderTester::SetEffectVariablesPerFrame()
{
	effects::kBasicEffect.SetEyePos(cam_.position_w_xf());
	effects::kBasicEffect.SetView(cam_.view());
	effects::kBasicEffect.SetShadowTransform(shadow_map_.world_to_tex());

	effects::kShadowMapEffect.SetLightViewProj(shadow_map_.view() * shadow_map_.proj());

	effects::kNormalDepthMapEffect.SetView(cam_.view());
}

void ba::FBXLoaderTester::SetEffectVariablesPerResize()
{
	effects::kBasicEffect.SetProj(cam_.proj());

	effects::kNormalDepthMapEffect.SetProj(cam_.proj());
}

void ba::FBXLoaderTester::SetEffectVariablesChangeRarely()
{
	XMMATRIX to_tex(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);

	effects::kBasicEffect.SetDirectionalLights(lights_);
	effects::kBasicEffect.SetFogStart(15.0f);
	effects::kBasicEffect.SetFogRange(175.0f);
	effects::kBasicEffect.SetFogColor(ba::color::kSilver);
	effects::kBasicEffect.SetShadowMapSize(kShadowMapSize);
	effects::kBasicEffect.SetToTex(to_tex);
	effects::kBasicEffect.SetShadowMap(shadow_map_.srv());
	effects::kBasicEffect.SetSSAOMap(ssao_map_.ssao_map_srv());

	// For object reflection.
	//effects::kBasicEffect.SetCubeMap();
}

void ba::FBXLoaderTester::UpdateOnKeyInput()
{
	// Control the camera.
	//
	if (key_pressed_['W'])
		cam_.MoveCameraZ(kForwardMovingRate * (float)timer_.get_delta_time());
	if (key_pressed_['S'])
		cam_.MoveCameraZ(-kForwardMovingRate * (float)timer_.get_delta_time());
	if (key_pressed_['D'])
		cam_.MoveCameraX(kRightMovingRate * (float)timer_.get_delta_time());
	if (key_pressed_['A'])
		cam_.MoveCameraX(-kRightMovingRate * (float)timer_.get_delta_time());
	if (key_pressed_['E'])
		cam_.MoveWorldY(kUpperMovingRate * (float)timer_.get_delta_time());
	if (key_pressed_['Q'])
		cam_.MoveWorldY(-kUpperMovingRate * (float)timer_.get_delta_time());
	//__
}

void ba::FBXLoaderTester::OnMouseMove(WPARAM w_par, int x, int y)
{
	if ((w_par & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(kRotationRate * static_cast<float>(x - last_mouse_pos_.x));
		float dy = XMConvertToRadians(kRotationRate * static_cast<float>(y - last_mouse_pos_.y));

		cam_.Pitch(dy);
		cam_.RotateWorldY(dx);
	}

	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;
}

void ba::FBXLoaderTester::OnMouseLBtnDown(WPARAM w_par, int x, int y)
{
	SetCapture(main_wnd_);
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;
}

void ba::FBXLoaderTester::OnMouseRBtnDown(WPARAM w_par, int x, int y)
{
	SetCapture(main_wnd_);
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;
}

void ba::FBXLoaderTester::OnMouseLBtnUp(WPARAM w_par, int x, int y)
{
	ReleaseCapture();
}

void ba::FBXLoaderTester::OnMouseRBtnUp(WPARAM w_par, int x, int y)
{
	ReleaseCapture();
}
