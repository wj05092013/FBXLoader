#include "stdafx.h"

ba::Mesh::Mesh() :
	vertex_stride_(0),
	vertex_count_(0),
	vb_(nullptr),
	transform_(XMMatrixIdentity())
{
}

ba::Mesh::~Mesh()
{
	ReleaseCOM(vb_);
}

void ba::Mesh::Draw(ID3D11DeviceContext* dc) const
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, &vb_, &vertex_stride_, &offset);

	dc->Draw(vertex_count_, 0);
}

void ba::Mesh::set_transform(const XMMATRIX& matrix)
{
	transform_ = matrix;
}

const DirectX::XMMATRIX& ba::Mesh::transform() const
{
	return transform_;
}

void ba::Mesh::set_material(const Material& material)
{
	material_ = material;
}

const ba::Material& ba::Mesh::material() const
{
	return material_;
}
