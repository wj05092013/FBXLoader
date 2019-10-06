#include "stdafx.h"

//
// Helper functions declarations
//

namespace ba
{
	static void ConvertMaterial(const FBXLoader::FBXLoaderMaterial& fbx_material, light::Material& out_material);
}


//
// Model class
//

bool ba::Model::Init(ID3D11Device* device, const FBXLoader::FBXLoaderModel& fbx_model)
{
	std::vector<inputvertex::PosNormalTexTangent::Vertex> vertices;
	light::Material material;

	const std::vector<FBXLoader::FBXLoaderMesh>& fbx_meshes = fbx_model.meshes;
	meshes.resize(fbx_meshes.size());

	for (UINT mesh_idx = 0; mesh_idx < meshes.size(); ++mesh_idx)
	{
		vertices.resize(fbx_meshes[mesh_idx].vertices.size());

		for (UINT vtx_idx = 0; vtx_idx < vertices.size(); ++vtx_idx)
		{
			vertices[vtx_idx].pos = fbx_meshes[mesh_idx].vertices[vtx_idx].pos;
			vertices[vtx_idx].normal = fbx_meshes[mesh_idx].vertices[vtx_idx].normal;
			vertices[vtx_idx].uv = fbx_meshes[mesh_idx].vertices[vtx_idx].uv;
			vertices[vtx_idx].tangent = fbx_meshes[mesh_idx].vertices[vtx_idx].tangent;
		}

		if (!meshes[mesh_idx].set_vertices(device, vertices))
		{
			meshes.clear();
			return false;
		}

		meshes[mesh_idx].set_transform(XMLoadFloat4x4(&fbx_meshes[mesh_idx].transform));

		ConvertMaterial(fbx_meshes[mesh_idx].material, material);
		meshes[mesh_idx].set_material(material);
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

void ba::ConvertMaterial(const FBXLoader::FBXLoaderMaterial& fbx_material, light::Material& out_material)
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
