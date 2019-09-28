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

bool ba::FBXLoader::Load(const std::string& filename, Model::EffectType effect_type, Model& out_model)
{
	// Create and initialize importer.
	//
	FbxImporter* importer = FbxImporter::Create(manager_, "");

	bool ret = importer->Initialize(filename.c_str(), -1, manager_->GetIOSettings());
	if (!ret)
	{
		importer->Destroy();
		return false;
	}
	//__

	// Create a scene and import on it.
	//
	FbxScene* scene = FbxScene::Create(manager_, "");

	ret = importer->Import(scene);
	if (!ret)
	{
		scene->Destroy();
		importer->Destroy();
		return false;
	}
	importer->Destroy();
	
	// Triangulate all nodes.
	FbxGeometryConverter geometry_converter(manager_);
	ret = geometry_converter.Triangulate(scene, true);
	if (!ret)
	{
		scene->Destroy();
		return false;
	}
	//__

	// Load scene.
	FbxNode* root_node = scene->GetRootNode();
	ret = Load(root_node, out_model);

	scene->Destroy();
	if (!ret)
	{
		return false;
	}

	// Set effect type.
	out_model.effect_type_ = effect_type;

	return true;
}

bool ba::FBXLoader::Load(FbxNode* node, Model& out_model)
{
	if (node)
	{
		if (node->GetNodeAttribute())
		{
			FbxNodeAttribute::EType attribute_type = node->GetNodeAttribute()->GetAttributeType();

			switch (attribute_type)
			{
			case fbxsdk::FbxNodeAttribute::eMesh:
				FbxMesh* fbx_mesh = node->GetMesh();

				if (!LoadMesh(fbx_mesh, out_model))
					return false;
				break;

			default:
				break;
			}
		}

		for (int i = 0; i < node->GetChildCount(); ++i)
		{
			if (!Load(node->GetChild(i), out_model))
				return false;
		}
	}

	return true;
}

bool ba::FBXLoader::LoadMesh(FbxMesh* fbx_mesh, Model& out_model)
{
	if (!fbx_mesh->IsTriangleMesh())
		return false;

	if (fbx_mesh->GetElementNormalCount() < 1)
		return false;
	if (fbx_mesh->GetElementTangentCount() < 1)
		return false;
	if (fbx_mesh->GetElementUVCount() < 1)
		return false;

	FbxVector4* control_points = fbx_mesh->GetControlPoints();

	int tri_count = fbx_mesh->GetPolygonCount();
	out_model.mesh_.vertices_.resize(3 * tri_count);

	for (int tri_idx = 0; tri_idx < tri_count; ++tri_idx)
	{
		for (int i = 0; i < 3; ++i)
		{
			int control_point_idx = fbx_mesh->GetPolygonVertex(tri_idx, i);
			int uv_idx = fbx_mesh->GetTextureUVIndex(tri_idx, i);
			int vertex_idx = 3 * tri_idx + i;

			ReadPosition(control_points[control_point_idx], out_model.mesh_.vertices_[vertex_idx].pos);
			if (!ReadNormal(fbx_mesh, control_point_idx, vertex_idx, out_model.mesh_.vertices_[vertex_idx].normal)) return false;
			if (!ReadTangent(fbx_mesh, control_point_idx, vertex_idx, out_model.mesh_.vertices_[vertex_idx].tangent)) return false;
			if (!ReadUV(fbx_mesh, control_point_idx, uv_idx, out_model.mesh_.vertices_[vertex_idx].uv)) return false;
		}
	}

	return true;
}

void ba::FBXLoader::ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos)
{
	out_pos.x = control_point[0];
	out_pos.y = control_point[1];
	out_pos.z = control_point[2];
}

bool ba::FBXLoader::ReadNormal(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal)
{
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
			return false;
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
			return false;
		}
		break;

	default:
		return false;
	}

	return true;
}

bool ba::FBXLoader::ReadTangent(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent)
{
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
			return false;
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
			return false;
		}
		break;

	default:
		return false;
	}

	return true;
}

bool ba::FBXLoader::ReadUV(FbxMesh* fbx_mesh, int control_point_idx, int uv_idx, XMFLOAT2& out_uv)
{
	const FbxGeometryElementUV* uv_element = fbx_mesh->GetElementUV();

	switch (uv_element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (uv_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			break;

		case FbxGeometryElement::eIndexToDirect:
			int idx = uv_element->GetIndexArray().GetAt(control_point_idx);
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[1]);
			break;

		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (uv_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(uv_idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(uv_idx).mData[1]);
			break;

		case FbxGeometryElement::eIndexToDirect:
			int idx = uv_element->GetIndexArray().GetAt(uv_idx);
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[1]);
			break;

		default:
			return false;
		}
		break;

	default:
		return false;
	}

	return true;
}
