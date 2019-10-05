#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Mesh
	{
	public:
		Mesh();
		~Mesh();

		void Draw(ID3D11DeviceContext* dc) const;

		template<class VertexType>
		bool set_vertices(ID3D11Device* device, const std::vector<VertexType> vertices);

		void set_transform(const XMMATRIX& matrix);
		const XMMATRIX& transform() const;

		void set_material(const Material& material);
		const Material& material() const;

	private:
		UINT vertex_stride_;
		UINT vertex_count_;
		ID3D11Buffer* vb_;

		XMMATRIX transform_;

		Material material_;
	};
}

template<class VertexType>
bool ba::Mesh::set_vertices(ID3D11Device* device, const std::vector<VertexType> vertices)
{
	ID3D11Buffer* old_vb = nullptr;
	if (vb_)
		old_vb = vb_;

	D3D11_BUFFER_DESC vb_desc{};
	vb_desc.ByteWidth = vertices.size() * sizeof(VertexType);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vb_init{};
	vb_init.pSysMem = &vertices[0];

	HRESULT res = device->CreateBuffer(&vb_desc, &vb_init, &vb_);
	if (FAILED(res))
	{
		vb_ = old_vb;
		return false;
	}

	ReleaseCOM(old_vb);
	vertex_stride_ = sizeof(VertexType);
	vertex_count_ = vertices.size();

	return true;
}