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

bool ba::FBXLoader::Load(const std::string& filename, FBXLoaderModel& out_model)
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

	out_model.meshes.resize(0);
	ret = LoadNode(root_node, out_model);

	scene->Destroy();
	if (!ret)
	{
		return false;
	}

	return true;
}

bool ba::FBXLoader::LoadNode(FbxNode* node, FBXLoaderModel& out_model)
{
	if (node)
	{
		for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
		{
			FbxNodeAttribute* attribute = node->GetNodeAttributeByIndex(i);
			FbxNodeAttribute::EType attribute_type = attribute->GetAttributeType();

			switch (attribute_type)
			{
			case fbxsdk::FbxNodeAttribute::eMesh:
			{
				FbxMesh* fbx_mesh = node->GetMesh();
				FBXLoaderMesh out_mesh;

				// Load mesh.
				if (!LoadGeometry(fbx_mesh, out_mesh))
					return false;

				// Get transform.
				LoadTransform(node, out_mesh.transform);

				LoadMaterial(node, out_mesh.material);

				out_model.meshes.push_back(out_mesh);

				break;
			}
			default:
				break;
			}
		}

		for (int i = 0; i < node->GetChildCount(); ++i)
		{
			if (!LoadNode(node->GetChild(i), out_model))
				return false;
		}
	}

	return true;
}

bool ba::FBXLoader::LoadGeometry(FbxMesh* fbx_mesh, FBXLoaderMesh& out_mesh)
{
	if (!fbx_mesh->IsTriangleMesh())
		return false;
	
	if (fbx_mesh->GetElementNormalCount() < 1)
		return false;
	if (fbx_mesh->GetElementUVCount() < 1)
		return false;
	if (fbx_mesh->GetElementTangentCount() < 1)
	{
		if (!fbx_mesh->GenerateTangentsDataForAllUVSets())
			return false;
	}

	// Load informations of vertices.
	//
	FbxVector4* control_points = fbx_mesh->GetControlPoints();

	int tri_count = fbx_mesh->GetPolygonCount();
	out_mesh.vertices.resize(3 * tri_count);

	for (int tri_idx = 0; tri_idx < tri_count; ++tri_idx)
	{
		for (int i = 0; i < 3; ++i)
		{
			int control_point_idx = fbx_mesh->GetPolygonVertex(tri_idx, i);
			int uv_idx = fbx_mesh->GetTextureUVIndex(tri_idx, i);
			int vertex_idx = 3 * tri_idx + i;

			ReadPosition(control_points[control_point_idx], out_mesh.vertices[vertex_idx].pos);
			if (!ReadNormal(fbx_mesh, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].normal)) return false;
			if (!ReadTangent(fbx_mesh, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].tangent)) return false;
			if (!ReadUV(fbx_mesh, control_point_idx, uv_idx, out_mesh.vertices[vertex_idx].uv)) return false;
		}
	}
	//__

	return true;
}

void ba::FBXLoader::LoadTransform(FbxNode* node, XMFLOAT4X4& out_transform)
{
	FbxAMatrix scale, rotation, translation;

	FbxVector4 vec = node->LclScaling.Get();
	scale.SetS(vec);

	vec = node->LclRotation.Get();
	rotation.SetR(vec);

	vec = node->LclTranslation.Get();
	translation.SetT(vec);

	FbxAMatrix transform = scale * rotation * translation;

	FbxAMatrixToXMFLOAT4x4(transform, out_transform);
}

void ba::FBXLoader::LoadMaterial(FbxNode* node, FBXLoaderMaterial& out_material)
{
	FbxSurfaceMaterial* fbx_material = node->GetMaterial(0);

	FbxProperty prop;
	FbxDouble3 color;
	FbxDouble factor;

	// Get ambient.
	//
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbient);
	color = prop.Get<FbxDouble3>();
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
	factor = prop.Get<FbxDouble>();

	out_material.ambient.x = static_cast<float>(color.mData[0]);
	out_material.ambient.y = static_cast<float>(color.mData[1]);
	out_material.ambient.z = static_cast<float>(color.mData[2]);

	out_material.ambient_factor = static_cast<float>(factor);
	//__

	// Get diffuse.
	//
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuse);
	color = prop.Get<FbxDouble3>();
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
	factor = prop.Get<FbxDouble>();

	out_material.diffuse.x = static_cast<float>(color.mData[0]);
	out_material.diffuse.y = static_cast<float>(color.mData[1]);
	out_material.diffuse.z = static_cast<float>(color.mData[2]);

	out_material.diffuse_factor = static_cast<float>(factor);
	//__

	// Get specular.
	//
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecular);
	color = prop.Get<FbxDouble3>();
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
	factor = prop.Get<FbxDouble>();

	out_material.specular.x = static_cast<float>(color.mData[0]);
	out_material.specular.y = static_cast<float>(color.mData[1]);
	out_material.specular.z = static_cast<float>(color.mData[2]);

	out_material.specular_factor = static_cast<float>(factor);
	//__

	// Get reflection.
	//
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sReflection);
	color = prop.Get<FbxDouble3>();
	prop = fbx_material->FindProperty(FbxSurfaceMaterial::sReflectionFactor);
	factor = prop.Get<FbxDouble>();

	out_material.reflection.x = static_cast<float>(color.mData[0]);
	out_material.reflection.y = static_cast<float>(color.mData[1]);
	out_material.reflection.z = static_cast<float>(color.mData[2]);

	out_material.reflection_factor = static_cast<float>(factor);
	//__
}

void ba::FBXLoader::ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos)
{
	out_pos.x = static_cast<float>(control_point[0]);
	out_pos.y = static_cast<float>(control_point[1]);
	out_pos.z = static_cast<float>(control_point[2]);
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
		{
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[2]);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int idx = normal_element->GetIndexArray().GetAt(control_point_idx);
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[2]);
			break;
		}
		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (normal_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[2]);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int idx = normal_element->GetIndexArray().GetAt(vertex_idx);
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(idx).mData[2]);
			break;
		}
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
		{
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[2]);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int idx = tangent_element->GetIndexArray().GetAt(control_point_idx);
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[2]);
			break;
		}
		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (tangent_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[2]);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int idx = tangent_element->GetIndexArray().GetAt(vertex_idx);
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(idx).mData[2]);
			break;
		}
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
		{
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int idx = uv_element->GetIndexArray().GetAt(control_point_idx);
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[1]);
			break;
		}
		default:
			return false;
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (uv_element->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(uv_idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(uv_idx).mData[1]);
			break;
		}
		case FbxGeometryElement::eIndexToDirect:
		{
			int idx = uv_element->GetIndexArray().GetAt(uv_idx);
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(idx).mData[1]);
			break;
		}
		default:
			return false;
		}
		break;

	default:
		return false;
	}

	return true;
}

void ba::FBXLoader::FbxAMatrixToXMFLOAT4x4(const FbxAMatrix& fbx_m, XMFLOAT4X4& out_m)
{
	out_m.m[0][0] = static_cast<float>(fbx_m.Get(0, 0));
	out_m.m[0][1] = static_cast<float>(fbx_m.Get(0, 1));
	out_m.m[0][2] = static_cast<float>(fbx_m.Get(0, 2));
	out_m.m[0][3] = static_cast<float>(fbx_m.Get(0, 3));

	out_m.m[1][0] = static_cast<float>(fbx_m.Get(1, 0));
	out_m.m[1][1] = static_cast<float>(fbx_m.Get(1, 1));
	out_m.m[1][2] = static_cast<float>(fbx_m.Get(1, 2));
	out_m.m[1][3] = static_cast<float>(fbx_m.Get(1, 3));

	out_m.m[2][0] = static_cast<float>(fbx_m.Get(2, 0));
	out_m.m[2][1] = static_cast<float>(fbx_m.Get(2, 1));
	out_m.m[2][2] = static_cast<float>(fbx_m.Get(2, 2));
	out_m.m[2][3] = static_cast<float>(fbx_m.Get(2, 3));

	out_m.m[3][0] = static_cast<float>(fbx_m.Get(3, 0));
	out_m.m[3][1] = static_cast<float>(fbx_m.Get(3, 1));
	out_m.m[3][2] = static_cast<float>(fbx_m.Get(3, 2));
	out_m.m[3][3] = static_cast<float>(fbx_m.Get(3, 3));
}
