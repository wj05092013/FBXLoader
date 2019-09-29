#include "stdafx.h"
#include "FBXLoaderTester.h"

const XMVECTOR ba::FBXLoaderTester::kInitCamPos = XMVectorSet(0.0f, 1.0f, -15.0f, 1.0f);
const XMVECTOR ba::FBXLoaderTester::kInitCamTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
const XMVECTOR ba::FBXLoaderTester::kInitCamUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

const float ba::FBXLoaderTester::kFovY = 0.25f * XM_PI;
const float ba::FBXLoaderTester::kNearZ = 0.1f;
const float ba::FBXLoaderTester::kFarZ = 1000.0f;

const float ba::FBXLoaderTester::kForwardMovingRate = 10.0f;
const float ba::FBXLoaderTester::kRightMovingRate = 10.0f;
const float ba::FBXLoaderTester::kUpperMovingRate = 10.0f;
const float ba::FBXLoaderTester::kRotationRate = 0.15f;

ba::FBXLoaderTester::FBXLoaderTester()
{
}

ba::FBXLoaderTester::~FBXLoaderTester()
{
}

void ba::FBXLoaderTester::Render()
{
	dc_->ClearRenderTargetView(rtv_, reinterpret_cast<const float*>(&ba::color::kMagenta));
	dc_->ClearDepthStencilView(dsv_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);

	swap_chain_->Present(0U, 0U);
}

void ba::FBXLoaderTester::UpdateDirectX()
{
}

bool ba::FBXLoaderTester::OnResize()
{
	if (!Application::OnResize())
		return false;

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

	return true;
}

void ba::FBXLoaderTester::ReleaseDirectX()
{
	effects::ReleaseAll();
	inputvertex::ReleaseAll();

	ReleaseDirectX();
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

void ba::FBXLoaderTester::SetEffectVariablesPerFrame()
{
	effects::kBasicEffect.SetEyePos(cam_.position_w_xf());
	effects::kBasicEffect.SetView(cam_.view());
	effects::kBasicEffect.SetShadowTransform();
	effects::kBasicEffect.SetShadowMap();
	effects::kBasicEffect.SetSSAOMap();
}

void ba::FBXLoaderTester::SetEffectVariablesPerResize()
{
	effects::kBasicEffect.SetProj(cam_.proj());
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
	effects::kBasicEffect.SetShadowMapSize();
	effects::kBasicEffect.SetToTex(to_tex);
	effects::kBasicEffect.SetCubeMap();
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
