#include "stdafx.h"

//
// Model class
//

bool ba::Model::Init(ID3D11Device* device, const FBXLoader::FBXLoaderModel& fbx_model)
{
	std::vector<inputvertex::PosNormalTexTangent::Vertex> vertices;

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
