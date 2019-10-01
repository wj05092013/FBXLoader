#include "stdafx.h"

ba::Mesh::Mesh() :
	vb_(nullptr),
	vertex_stride_(0)
{
}

void ba::Mesh::Draw(ID3D11DeviceContext* dc) const
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, &vb_, &vertex_stride_, &offset);

	dc->Draw(vertices_.size(), 0);
}

void ba::Mesh::set_material(const Material& material)
{
	material_ = material;
}

const ba::Material& ba::Mesh::material() const
{
	return material_;
}

bool ba::Mesh::Init(ID3D11Device* device)
{
	if (!BuildGeometryBuffer(device))
		return false;

	return true;
}

void ba::Mesh::Release()
{
	ReleaseCOM(vb_);
	vertices_.clear();
	vertex_stride_ = 0;
}

bool ba::Mesh::BuildGeometryBuffer(ID3D11Device* device)
{
	if (vertices_.size() < 3)
	{
		return false;
	}

	vertex_stride_ = sizeof(inputvertex::PosNormalTexTangent);

	D3D11_BUFFER_DESC vb_desc{};
	vb_desc.ByteWidth = vertices_.size() * vertex_stride_;
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vb_init{};
	vb_init.pSysMem = &vertices_[0];

	HRESULT res = device->CreateBuffer(&vb_desc, &vb_init, &vb_);
	if (FAILED(res))
	{
		vertex_stride_ = 0;
		return false;
	}

	return true;
}