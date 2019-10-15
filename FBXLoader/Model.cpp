#include "stdafx.h"

//
// Helper functions declarations
//

namespace ba
{
	static void StoreVertex(const FBXLoadedVertex& fbx_vertex, inputvertex::PosNormalTexTangent::Vertex& out_vertex);
	static void StoreVertex(const std::vector<FBXLoadedVertex>& fbx_vertices, std::vector<inputvertex::PosNormalTexTangent::Vertex>& out_vertices);

	static void StoreTangent(const std::vector<FBXLoadedVertex>& fbx_vertices, std::vector<inputvertex::PosNormalTexTangent::Vertex>& out_vertices);

	static void StoreMaterial(const FBXLoadedMaterial& fbx_material, light::Material& out_material);
	static void StoreMaterial(const std::vector<FBXLoadedMaterial>& fbx_materials, std::vector<light::Material>& out_materials);

	static void CreateTexSRV(ID3D11Device* device, ID3D11DeviceContext* dc, const std::vector<FBXLoadedTextureInfo> tex_infos, ID3D11ShaderResourceView** srv);
}


//
// Model class
//

bool ba::Model::Init(ID3D11Device* device, const FBXLoadedModel& fbx_model)
{
	std::vector<inputvertex::PosNormalTexTangent::Vertex> vertices;
	std::vector<int> material_indices(0);

	const std::vector<FBXLoadedMesh>& fbx_meshes = fbx_model.meshes;
	meshes.resize(fbx_meshes.size());

	for (UINT mesh_idx = 0; mesh_idx < meshes.size(); ++mesh_idx)
	{
		// Convert 'FBXLoadedVertex's to 'PosNormalTexTangent::Vertex's.
		StoreVertex(fbx_meshes[mesh_idx].vertices, vertices);

		meshes[mesh_idx].set_b_has_tangent(fbx_meshes[mesh_idx].b_has_tangent);
		if (meshes[mesh_idx].b_has_tangent())
			StoreTangent(fbx_meshes[mesh_idx].vertices, vertices);

		// Build VB with the converted vertices.
		if (!meshes[mesh_idx].BuildVertexBuffer(device, vertices))
		{
			meshes.clear();
			return false;
		}

		// Get face material indices.
		material_indices.insert(material_indices.begin(), fbx_meshes[mesh_idx].face_material_indices.begin(), fbx_meshes[mesh_idx].face_material_indices.end());

		// Build a srv for the face material indices.
		if (!meshes[mesh_idx].BuildFaceMaterialIndicesView(device, material_indices))
		{
			meshes.clear();
			return false;
		}

		// Convert 'FBXLoadedMaterial's to 'light::Material's.
		StoreMaterial(fbx_meshes[mesh_idx].materials, meshes[mesh_idx].materials);

		// Get transform matrix.
		meshes[mesh_idx].set_transform(XMLoadFloat4x4(&fbx_meshes[mesh_idx].transform));
	}

	return true;
}


//
// ModelInstance class
//

ba::ModelInstance::ModelInstance() :
	model(nullptr),
	scale(XMMatrixIdentity()),
	rotation(XMMatrixIdentity()),
	translation(XMMatrixIdentity())
{
}


//
// Helper functions definitions
//

void ba::StoreVertex(const FBXLoadedVertex& fbx_vertex, inputvertex::PosNormalTexTangent::Vertex& out_vertex)
{
	out_vertex.pos = fbx_vertex.pos;
	out_vertex.normal = fbx_vertex.normal;
	out_vertex.uv = fbx_vertex.uv;
	//out_vertex.tangent = fbx_vertex.tangent;
}

void ba::StoreVertex(const std::vector<FBXLoadedVertex>& fbx_vertices, std::vector<inputvertex::PosNormalTexTangent::Vertex>& out_vertices)
{
	out_vertices.resize(fbx_vertices.size());

	for (UINT vtx_idx = 0; vtx_idx < fbx_vertices.size(); ++vtx_idx)
	{
		StoreVertex(fbx_vertices[vtx_idx], out_vertices[vtx_idx]);
	}
}

void ba::StoreTangent(const std::vector<FBXLoadedVertex>& fbx_vertices, std::vector<inputvertex::PosNormalTexTangent::Vertex>& out_vertices)
{
	for (int i = 0; i < fbx_vertices.size(); ++i)
		out_vertices[i].tangent = fbx_vertices[i].tangent;
}

void ba::StoreMaterial(const FBXLoadedMaterial& fbx_material, light::Material& out_material)
{
	out_material.ambient = XMFLOAT4(
		fbx_material.ambient.x,
		fbx_material.ambient.y,
		fbx_material.ambient.z,
		1.0f
	);

	out_material.diffuse = XMFLOAT4(
		fbx_material.diffuse.x,
		fbx_material.diffuse.y,
		fbx_material.diffuse.z,
		1.0f
	);

	out_material.specular = XMFLOAT4(
		fbx_material.specular.x,
		fbx_material.specular.y,
		fbx_material.specular.z,
		fbx_material.shininess
	);

	out_material.reflection = XMFLOAT4(
		fbx_material.reflection.x,
		fbx_material.reflection.y,
		fbx_material.reflection.z,
		1.0f
	);
}

void ba::StoreMaterial(const std::vector<FBXLoadedMaterial>& fbx_materials, std::vector<light::Material>& out_materials)
{
	out_materials.resize(fbx_materials.size());

	for (int i = 0; i < fbx_materials.size(); ++i)
	{
		StoreMaterial(fbx_materials[i], out_materials[i]);
	}
}

void ba::CreateTexSRV(ID3D11Device* device, ID3D11DeviceContext* dc, const std::vector<FBXLoadedTextureInfo> tex_infos, ID3D11ShaderResourceView** srv)
{
	for (int i = 0; i < tex_infos.size(); ++i)
	{
		std::wstring file_name(tex_infos[i].file_name.());
		std::wstring file_path(L"Texture/" + tex_infos[i].file_name);

		D3DX11CreateShaderResourceViewFromFile(device, file_path.c_str(), )
	}
}
