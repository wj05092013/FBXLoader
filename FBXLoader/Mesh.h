#pragma once

/*
PCH: Yes
*/

namespace ba
{
	struct Mesh
	{
		ID3D11Buffer* vb;
		UINT vertex_stride;

		std::vector<inputvertex::PosNormalTexTangent::Vertex> vertices;
	};
}