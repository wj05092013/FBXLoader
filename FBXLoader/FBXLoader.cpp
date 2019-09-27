#include "stdafx.h"

ba::FBXLoader::FBXLoader() :
	manager_(nullptr),
	io_settings_(nullptr)
{

}

ba::FBXLoader& ba::FBXLoader::GetInstance()
{
	static FBXLoader instance;
	return instance;
}

void ba::FBXLoader::Init()
{
	if (manager_ == nullptr)
	{
		manager_ = FbxManager::Create();
		io_settings_ = FbxIOSettings::Create(manager_, IOSROOT);
		manager_->SetIOSettings(io_settings_);
	}
}

void ba::FBXLoader::Release()
{
	if (manager_)
	{
		manager_->Destroy();
		manager_ = nullptr;
		
		io_settings_ = nullptr;
	}
}

bool ba::FBXLoader::Load(const std::string& filename)
{
	// Create and initialize importer.
	//
	FbxImporter* importer = FbxImporter::Create(manager_, "");

	bool ret = importer->Initialize(filename.c_str(), -1, manager_->GetIOSettings());
	if (!ret) { Release(); return false; }
	//__

	// Create a scene and import on it.
	//
	FbxScene* scene = FbxScene::Create(manager_, "");

	ret = importer->Import(scene);
	if (!ret) { Release(); return false; }
	
	// Triangulate all nodes.
	FbxGeometryConverter geometry_converter(manager_);
	ret = geometry_converter.Triangulate(scene, true);
	if (!ret) { Release(); return false; }

	importer->Destroy();
	//__

	FbxNode* root_node = scene->GetRootNode();
	Load(root_node);

	return true;
}

void ba::FBXLoader::Load(FbxNode* node)
{
	if (node)
	{
		if (node->GetNodeAttribute())
		{
			FbxNodeAttribute::EType attribute_type = node->GetNodeAttribute()->GetAttributeType();

			switch (attribute_type)
			{
			case fbxsdk::FbxNodeAttribute::eMesh:
				FbxMesh* mesh = node->GetMesh();
				LoadMesh(mesh);
				break;

			default:
				break;
			}
		}

		for (int i = 0; i < node->GetChildCount(); ++i)
		{
			Load(node->GetChild(i));
		}
	}
}

void ba::FBXLoader::LoadMesh(FbxMesh* mesh)
{
	// Output mesh data. These have to be replaced by real output data.
	//
	std::vector<inputvertex::PosNormalTex::Vertex> vertices;
	std::vector<UINT> indices;
	//__

	LoadVertices(mesh, vertices);
	LoadIndices(mesh, indices);
}

void ba::FBXLoader::LoadVertices(FbxMesh* mesh, std::vector<ba::inputvertex::PosNormalTex::Vertex>& out_vertices)
{
	int control_point_count = mesh->GetControlPointsCount();

	out_vertices.resize(control_point_count);

	for (UINT i = 0; i < control_point_count; ++i)
	{
		out_vertices[i].pos.x = static_cast<float>(mesh->GetControlPointAt(i)[0]);
		out_vertices[i].pos.y = static_cast<float>(mesh->GetControlPointAt(i)[1]);
		out_vertices[i].pos.z = static_cast<float>(mesh->GetControlPointAt(i)[2]);
	}
}

void ba::FBXLoader::LoadIndices(FbxMesh* mesh, std::vector<UINT>& out_indices)
{
	int tri_count = mesh->GetPolygonCount();
	out_indices.resize(3 * tri_count);

	UINT index_count = 0;
	for (int tri_idx = 0; tri_idx < tri_count; ++tri_idx)
	{
		for (int i = 0; i < 3; ++i)
		{
			out_indices[index_count] = mesh->GetPolygonVertex(tri_idx, i);
			++index_count;
		}
	}
}
