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

	static bool CreateSRVFromFBXLoadedTextureInfo(ID3D11Device* device, ID3D11DeviceContext* dc, const std::vector<FBXLoadedTextureInfo> tex_infos, ID3D11ShaderResourceView** out_srv);
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
		// Comment out this part because of an error.
		/*if (!meshes[mesh_idx].BuildFaceMaterialIndicesView(device, material_indices))
		{
			meshes.clear();
			return false;
		}*/

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
	for (UINT i = 0; i < fbx_vertices.size(); ++i)
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

	for (UINT i = 0; i < fbx_materials.size(); ++i)
	{
		StoreMaterial(fbx_materials[i], out_materials[i]);
	}
}

bool ba::CreateSRVFromFBXLoadedTextureInfo(ID3D11Device* device, ID3D11DeviceContext* dc, const std::vector<FBXLoadedTextureInfo> tex_infos, ID3D11ShaderResourceView** out_srv)
{
	// Create rtv and srv for the base texture.
	//
	ID3D11RenderTargetView* base_tex_rtv = nullptr;
	ID3D11ShaderResourceView* base_tex_srv = nullptr;

	std::vector<UCHAR> init_data(256 * 256 * 4, 0);
	if (!CreateTexRTVAndSRV(device, 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM, &base_tex_rtv, &base_tex_srv))
		return false;
	//__

	// Create srvs for all textures represented as the instances of 'FBXLoadedTextureInfo'.
	//
	std::vector<ID3D11ShaderResourceView*> srvs(tex_infos.size(), nullptr);

	for (UINT srv_idx = 0; srv_idx < srvs.size(); ++srv_idx)
	{
		std::wstring file_name;
		file_name.assign(tex_infos[srv_idx].file_name.begin(), tex_infos[srv_idx].file_name.end());
		file_name = L"Texture/" + file_name;

		// Fail to create srv for this texture.
		if (!TextureManager::GetInstance().CreateSRV(file_name, &srvs[srv_idx]))
		{
			ReleaseCOM(base_tex_rtv);
			ReleaseCOM(base_tex_srv);
			return false;
		}
	}
	//__

	// Blend all textures.
	//

	//__

	ReleaseCOM(base_tex_rtv);
	*out_srv = base_tex_srv;

	return true;
}
