#pragma once

/*
PCH: Yes
*/

namespace ba
{
	template<class VertexType>
	class Mesh
	{
	public:
		Mesh();

		bool Init(ID3D11Device* device, const std::vector<VertexType>& vertices);
		void Release();

		void Draw(ID3D11DeviceContext* dc);

	private:
		bool BuildGeometryBuffer(ID3D11Device* device);

		std::vector<VertexType> vertices_;

		ID3D11Buffer* vb_;
		UINT vertex_stride_;

		// Add materials.
	};
}

template<class VertexType>
ba::Mesh<VertexType>::Mesh() :
	vb_(nullptr),
	vertex_stride_(0)
{
}

template<class VertexType>
void ba::Mesh<VertexType>::Draw(ID3D11DeviceContext* dc)
{
	UINT offset = 0;

	dc->IASetVertexBuffers(0, 1, &vb_, &vertex_stride_, &offset);

	dc->Draw(vertices_.size(), 0);
}

template<class VertexType>
bool ba::Mesh<VertexType>::Init(ID3D11Device* device, const std::vector<VertexType>& vertices)
{
	vertices_.resize(0);
	vertices_.insert(vertices_.begin(), vertices.begin(), vertices.end());

	if (!BuildGeometryBuffer<VertexType>(device))
		return false;

	return true;
}

template<class VertexType>
inline void ba::Mesh<VertexType>::Release()
{
	ReleaseCOM(vb_);
	vertices_.resize(0);
	vertex_stride_ = 0;
}

template<class VertexType>
bool ba::Mesh<class VertexType>::BuildGeometryBuffer(ID3D11Device* device)
{
	if (vertices_.size() < 3)
	{
		return false;
	}

	vertex_stride_ = sizeof(VertexType);

	D3D11_BUFFER_DESC vb_desc{};
	vb_desc.ByteWidth = vertices_.size() * vertex_stride_;
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vb_init{};
	vb_init.pSysMem = &vertices_[0];

	HRESULT res = device->CreateBuffer(&vb_desc, &vb_init, &vb_);
	if (FAILED(res))
		return false;

	return true;
}