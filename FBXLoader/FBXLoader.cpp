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

bool ba::FBXLoader::Load(const std::string& filename, Mesh& out_mesh)
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
	Load(root_node, out_mesh);

	return true;
}

void ba::FBXLoader::Load(FbxNode* node, Mesh& out_mesh)
{
	if (node)
	{
		if (node->GetNodeAttribute())
		{
			FbxNodeAttribute::EType attribute_type = node->GetNodeAttribute()->GetAttributeType();

			switch (attribute_type)
			{
			case fbxsdk::FbxNodeAttribute::eMesh:
				const FbxMesh* fbx_mesh = node->GetMesh();

				LoadMesh(fbx_mesh, out_mesh);

				break;

			default:
				break;
			}
		}

		for (int i = 0; i < node->GetChildCount(); ++i)
		{
			Load(node->GetChild(i), out_mesh);
		}
	}
}

void ba::FBXLoader::LoadMesh(const FbxMesh* fbx_mesh, Mesh& out_mesh)
{
	FbxVector4* control_points = fbx_mesh->GetControlPoints();

	int tri_count = fbx_mesh->GetPolygonCount();
	out_mesh.vertices.resize(3 * tri_count);

	for (int tri_idx = 0; tri_idx < tri_count; ++tri_idx)
	{
		for (int i = 0; i < 3; ++i)
		{
			int control_point_idx = fbx_mesh->GetPolygonVertex(tri_idx, i);
			int vertex_idx = 3 * tri_idx + i;

			ReadPosition(control_points[control_point_idx], out_mesh.vertices[vertex_idx].pos);
			ReadNormal(fbx_mesh, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].normal);
			ReadTangent(fbx_mesh, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].tangent);
			
			// Add some codes to read uv coordinates.
		}
	}

}

void ba::FBXLoader::ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos)
{
	out_pos.x = control_point[0];
	out_pos.y = control_point[1];
	out_pos.z = control_point[2];
}

void ba::FBXLoader::ReadNormal(const FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal)
{
	if (fbx_mesh->GetElementNormalCount() < 1)
		return;

	const FbxGeometryElementNormal* normal_element = fbx_mesh->GetElementNormal();

	switch (normal_element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (normal_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[2]);
			break;

		case FbxGeometryElement::eIndexToDirect:
			int idx = normal_element->GetIndexArray().GetAt(control_point_idx);
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[2]);
			break;

		default:
			break;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (normal_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[2]);
			break;

		case FbxGeometryElement::eIndexToDirect:
			int idx = normal_element->GetIndexArray().GetAt(vertex_idx);
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[2]);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void ba::FBXLoader::ReadTangent(const FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent)
{
	if (fbx_mesh->GetElementTangentCount() < 1)
		return;

	const FbxGeometryElementTangent* tangent_element = fbx_mesh->GetElementTangent();

	switch (tangent_element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (tangent_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[2]);
			break;

		case FbxGeometryElement::eIndexToDirect:
			int idx = tangent_element->GetIndexArray().GetAt(control_point_idx);
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[2]);
			break;

		default:
			break;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (tangent_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[2]);
			break;

		case FbxGeometryElement::eIndexToDirect:
			int idx = tangent_element->GetIndexArray().GetAt(vertex_idx);
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[2]);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}
