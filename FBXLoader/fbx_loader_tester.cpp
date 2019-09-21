#include "stdafx.h"
#include "fbx_loader_tester.h"

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

	return true;
}

void ba::FBXLoaderTester::ReleaseDirectX()
{


	ReleaseDirectX();
}
