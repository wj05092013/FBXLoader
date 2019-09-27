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

	FbxVector4* control_points = mesh->GetControlPoints();

	int tri_count = mesh->GetPolygonCount();
	vertices.resize(3 * tri_count);

	for (int tri_idx = 0; tri_idx < tri_count; ++tri_idx)
	{
		inputvertex::PosNormalTex::Vertex vertex;

		for (int i = 0; i < 3; ++i)
		{
			int control_point_idx = mesh->GetPolygonVertex(tri_idx, i);

		}
	}

}
