#include "stdafx.h"

//
// Helper functions declaration
//

namespace ba
{
	static void FbxAMatrixToXMFLOAT4x4(const FbxAMatrix& fbx_m, XMFLOAT4X4& out_m);
}


//
// FBXLoader class
//

const int ba::FBXLoader::kMaterialCountMax = 16;

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
	//
	FbxNode* root_node = scene->GetRootNode();

	out_model.meshes.resize(0);

	//  Load all nodes recursively.
	ret = LoadNode(root_node, out_model);

	scene->Destroy();
	if (!ret)
	{
		return false;
	}
	//__

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

				// Load geometry data.
				if (!LoadGeometry(fbx_mesh, out_mesh))
					return false;

				// Get transform.
				LoadTransform(node, out_mesh.transform);

				// Get materials
				LoadMaterials(node, out_mesh.materials);

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

	FbxLayer* base_layer = fbx_mesh->GetLayer(0);
	
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
			if (!ReadNormal(base_layer, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].normal)) return false;
			if (!ReadUV(base_layer, control_point_idx, uv_idx, out_mesh.vertices[vertex_idx].uv)) return false;
			if (!ReadMaterialIndex(base_layer, tri_idx, out_mesh.face_material_indices[tri_idx])) return false;
			if (!ReadTangent(base_layer, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].tangent)) return false;
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

void ba::FBXLoader::LoadMaterials(FbxNode* node, std::vector<FBXLoaderMaterial>& out_materials)
{
	int material_count = node->GetMaterialCount();
	if (material_count < 1)
		return;

	out_materials.resize(material_count);

	for (int i = 0; i < material_count; ++i)
	{
		FbxSurfaceMaterial* fbx_material = node->GetMaterial(i);

		// Test
		//
		fbx_material->GetNameOnly();
		//

		FbxProperty prop;
		FbxDouble3 color;
		FbxDouble factor;

		// Get ambient.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbient);
		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[i].ambient.x = static_cast<float>(color.mData[0]);
		out_materials[i].ambient.y = static_cast<float>(color.mData[1]);
		out_materials[i].ambient.z = static_cast<float>(color.mData[2]);
		//__

		// Get diffuse.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuse);

		//  Get diffuse map for this material.
		//
		int layered_tex_count = prop.GetSrcObjectCount<FbxLayeredTexture>();
		if (layered_tex_count > 0)
		{

		}

		for (int i = 0; i < prop.GetSrcObjectCount(); ++i)
		{
			FbxFileTexture* tex = prop.GetSrcObject<FbxFileTexture>(i);
			std::string name = tex->GetName();
		}
		// __

		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[i].diffuse.x = static_cast<float>(color.mData[0]);
		out_materials[i].diffuse.y = static_cast<float>(color.mData[1]);
		out_materials[i].diffuse.z = static_cast<float>(color.mData[2]);
		//__

		// Get specular.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecular);
		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[i].specular.x = static_cast<float>(color.mData[0]);
		out_materials[i].specular.y = static_cast<float>(color.mData[1]);
		out_materials[i].specular.z = static_cast<float>(color.mData[2]);
		//__

		// Get reflection.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sReflection);
		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sReflectionFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[i].reflection.x = static_cast<float>(color.mData[0]);
		out_materials[i].reflection.y = static_cast<float>(color.mData[1]);
		out_materials[i].reflection.z = static_cast<float>(color.mData[2]);
		//__

		// Get shininess.
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sShininess);
		FbxDouble shininess = prop.Get<FbxDouble>();

		out_materials[i].shininess = static_cast<float>(shininess);
		//__
	}
}

void ba::FBXLoader::ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos)
{
	out_pos.x = static_cast<float>(control_point[0]);
	out_pos.y = static_cast<float>(control_point[1]);
	out_pos.z = static_cast<float>(control_point[2]);
}

bool ba::FBXLoader::ReadNormal(FbxLayer* layer, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal)
{
	const FbxLayerElementNormal* normal_element = layer->GetNormals();
	if (!normal_element)
		return false;

	switch (normal_element->GetMappingMode())
	{
	case FbxLayerElement::eByControlPoint:
		switch (normal_element->GetReferenceMode())
		{
		case FbxLayerElement::eDirect:
		{
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(control_point_idx).mData[2]);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
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

	case FbxLayerElement::eByPolygonVertex:
		switch (normal_element->GetReferenceMode())
		{
		case FbxLayerElement::eDirect:
		{
			out_normal.x = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[0]);
			out_normal.y = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[1]);
			out_normal.z = static_cast<float>(normal_element->GetDirectArray().GetAt(vertex_idx).mData[2]);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
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

bool ba::FBXLoader::ReadTangent(FbxLayer* layer, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent)
{
	const FbxLayerElementTangent* tangent_element = layer->GetTangents();
	if (!tangent_element)
	{
		if (!layer->CreateLayerElementOfType(FbxLayerElement::EType::eTangent))
			return false;
	}

	switch (tangent_element->GetMappingMode())
	{
	case FbxLayerElement::eByControlPoint:
		switch (tangent_element->GetReferenceMode())
		{
		case FbxLayerElement::eDirect:
		{
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(control_point_idx).mData[2]);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
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

	case FbxLayerElement::eByPolygonVertex:
		switch (tangent_element->GetReferenceMode())
		{
		case FbxLayerElement::eDirect:
		{
			out_tangent.x = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[0]);
			out_tangent.y = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[1]);
			out_tangent.z = static_cast<float>(tangent_element->GetDirectArray().GetAt(vertex_idx).mData[2]);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
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

bool ba::FBXLoader::ReadUV(FbxLayer* layer, int control_point_idx, int uv_idx, XMFLOAT2& out_uv)
{
	const FbxLayerElementUV* uv_element = layer->GetUVs();
	if (!uv_element)
		return false;

	switch (uv_element->GetMappingMode())
	{
	case FbxLayerElement::eByControlPoint:
		switch (uv_element->GetReferenceMode())
		{
		case FbxLayerElement::eDirect:
		{
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(control_point_idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(control_point_idx).mData[1]);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
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

	case FbxLayerElement::eByPolygonVertex:
		switch (uv_element->GetReferenceMode())
		{
		case FbxLayerElement::eDirect:
		{
			out_uv.x = static_cast<float>(uv_element->GetDirectArray().GetAt(uv_idx).mData[0]);
			out_uv.y = static_cast<float>(uv_element->GetDirectArray().GetAt(uv_idx).mData[1]);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
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

bool ba::FBXLoader::ReadMaterialIndex(FbxLayer* layer, int triangle_idx, int& out_material_idx)
{
	const FbxLayerElementMaterial* material_element = layer->GetMaterials();
	if (!material_element)
		return false;

	switch (material_element->GetMappingMode())
	{
	case FbxLayerElement::eAllSame:
	{
		// material index is stored in the IndexArray, not the DirectArray.
		out_material_idx = material_element->GetIndexArray().GetAt(0);
		break;
	}
	case FbxLayerElement::eByPolygon:
	{
		out_material_idx = material_element->GetIndexArray().GetAt(triangle_idx);
		break;
	}
	default:
		return false;
	}
	return true;
}


//
// Helper functions definitions
//

void ba::FbxAMatrixToXMFLOAT4x4(const FbxAMatrix& fbx_m, XMFLOAT4X4& out_m)
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
