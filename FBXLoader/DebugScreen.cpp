#include "stdafx.h"

ba::DebugScreen::DebugScreen() :
	vb_(nullptr),
	ib_(nullptr),
	srv_(nullptr)
{
}

ba::DebugScreen::~DebugScreen()
{
	Release();
}

bool ba::DebugScreen::Init(ID3D11Device* device)
{
	if (vb_ || ib_)
		return false;

	if (!BuildGeometryBuffers(device))
		return false;

	return true;
}

void ba::DebugScreen::Release()
{
	ReleaseCOM(vb_);
	ReleaseCOM(ib_);
}

void ba::DebugScreen::Render(ID3D11DeviceContext* dc)
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dc->IASetInputLayout(inputvertex::PosNormalTex::kInputLayout);

	UINT strides = sizeof(inputvertex::PosNormalTex::Vertex);
	UINT offsets = 0;
	dc->IASetVertexBuffers(0, 1, &vb_, &strides, &offsets);
	dc->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);

	ID3DX11EffectTechnique* tech = effects::kDebugScreenEffect.tech(DebugScreenEffect::kViewRed);
	D3DX11_TECHNIQUE_DESC tech_desc;
	tech->GetDesc(&tech_desc);

	effects::kDebugScreenEffect.SetProj(XMLoadFloat4x4(&proj_));
	effects::kDebugScreenEffect.SetDebugTex(srv_);

	for (UINT p = 0; p < tech_desc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(6, 0, 0);
	}
}

void ba::DebugScreen::set_ndc_position_size(float left_top_x, float left_top_y, float width, float height)
{
	XMMATRIX proj(
		0.5f * width, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f * height, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		left_top_x + 0.5f * width, left_top_y - 0.5f * height, 0.0f, 1.0f
	);
	XMStoreFloat4x4(&proj_, proj);
}

void ba::DebugScreen::set_srv(ID3D11ShaderResourceView* srv)
{
	srv_ = srv;
}

bool ba::DebugScreen::BuildGeometryBuffers(ID3D11Device* device)
{
	GeometryGenerator::Mesh mesh;
	GeometryGenerator::CreateFullscreenQuad(mesh);

	std::vector<inputvertex::PosNormalTex::Vertex> vertices(mesh.vertices.size());

	for (UINT i = 0; i < vertices.size(); ++i)
	{
		vertices[i].pos = mesh.vertices[i].pos;
		vertices[i].normal = mesh.vertices[i].normal;
		vertices[i].uv = mesh.vertices[i].uv;
	}

	D3D11_BUFFER_DESC vb_desc{};
	vb_desc.ByteWidth = sizeof(inputvertex::PosNormalTex::Vertex) * vertices.size();
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vb_init{};
	vb_init.pSysMem = &vertices[0];
	HRESULT res = device->CreateBuffer(&vb_desc, &vb_init, &vb_);
	if (FAILED(res))
		return false;

	D3D11_BUFFER_DESC ib_desc{};
	ib_desc.ByteWidth = sizeof(UINT) * mesh.indices.size();
	ib_desc.Usage = D3D11_USAGE_IMMUTABLE;
	ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA ib_init{};
	ib_init.pSysMem = &mesh.indices[0];
	res = device->CreateBuffer(&ib_desc, &ib_init, &ib_);
	if (FAILED(res))
	{
		ReleaseCOM(vb_);
		return false;
	}

	return true;
}
