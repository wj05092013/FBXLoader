#include "stdafx.h"

//
// Mesh
//

ba::Mesh::Mesh() :
	vb_(nullptr),
	vertex_stride_(0),
	vertex_count_(0),
	transform_(XMMatrixIdentity()),
	b_has_tangent_(false),
	material_indices_view_(nullptr)
{
}

ba::Mesh::~Mesh()
{
	ReleaseCOM(vb_);
	ReleaseCOM(material_indices_view_);
}

void ba::Mesh::Draw(ID3D11DeviceContext* dc) const
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, &vb_, &vertex_stride_, &offset);

	dc->Draw(vertex_count_, 0);
}

bool ba::Mesh::BuildFaceMaterialIndicesView(ID3D11Device* device, const std::vector<int>& material_indices)
{
	if (material_indices_view_)
		return false;

	D3D11_TEXTURE1D_DESC tex_desc{};
	tex_desc.Width = sizeof(int) * material_indices.size();
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_SINT;
	tex_desc.Usage = D3D11_USAGE_IMMUTABLE;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	
	D3D11_SUBRESOURCE_DATA tex_init{};
	tex_init.pSysMem = &material_indices[0];

	ID3D11Texture1D* tex = nullptr;
	HRESULT res = device->CreateTexture1D(&tex_desc, &tex_init, &tex);
	if (FAILED(res))
		return false;

	res = device->CreateShaderResourceView(tex, nullptr, &material_indices_view_);
	ReleaseCOM(tex);
	if (FAILED(res))
		return false;

	return true;
}

void ba::Mesh::set_transform(const XMMATRIX& matrix)
{
	transform_ = matrix;
}

const DirectX::XMMATRIX& ba::Mesh::transform() const
{
	return transform_;
}

void ba::Mesh::set_b_has_tangent(bool b_has)
{
	b_has_tangent_ = b_has;
}

bool ba::Mesh::b_has_tangent() const
{
	return b_has_tangent_;
}

ID3D11ShaderResourceView* ba::Mesh::material_indices_view() const
{
	return material_indices_view_;
}


//
// IndexedMesh
//

ba::IndexedMesh::IndexedMesh() :
	ib_(nullptr),
	idx_count_(0)
{
}

ba::IndexedMesh::~IndexedMesh()
{
}

void ba::IndexedMesh::Draw(ID3D11DeviceContext* dc) const
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, &vb_, &vertex_stride_, &offset);
	dc->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);

	dc->DrawIndexed(idx_count_, 0, 0);
}

bool ba::IndexedMesh::set_indices(ID3D11Device* device, const std::vector<UINT> indices)
{
	ID3D11Buffer* old_ib = ib_;

	D3D11_BUFFER_DESC ib_desc{};
	ib_desc.ByteWidth = indices.size() * sizeof(UINT);
	ib_desc.Usage = D3D11_USAGE_IMMUTABLE;
	ib_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ib_init{};
	ib_init.pSysMem = &indices[0];

	HRESULT res = device->CreateBuffer(&ib_desc, &ib_init, &ib_);
	if (FAILED(res))
	{
		ib_ = old_ib;
		return false;
	}

	ReleaseCOM(old_ib);
	idx_count_ = indices.size();

	return true;
}
