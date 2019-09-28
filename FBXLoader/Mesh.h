#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Mesh : public Uncopiable
	{
	public:
		Mesh();

		bool Init(ID3D11Device* device);
		void Release();

		void Draw(ID3D11DeviceContext* dc);

	private:
		bool BuildGeometryBuffer(ID3D11Device* device);

	public:
		std::vector<inputvertex::PosNormalTexTangent::Vertex> vertices_;

	private:
		ID3D11Buffer* vb_;
		UINT vertex_stride_;

		// Add materials.
	};
}
