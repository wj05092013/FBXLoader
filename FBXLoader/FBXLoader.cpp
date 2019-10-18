#include "stdafx.h"

//
// Helper functions declaration
//

namespace ba
{
	static void FbxAMatrixToXMFLOAT4x4(const FbxAMatrix& fbx_m, XMFLOAT4X4& out_m);
	static bool LoadTextureInfo(const FbxTexture* fbx_tex, FbxTexture::ETextureUse tex_use, FBXLoadedTextureInfo& out_info);
}


//
// FBXLoader class
//

ba::FBXLoader::FBXLoader() :
	manager_(nullptr),
	io_settings_(nullptr),
	mesh_counter_(0)
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

bool ba::FBXLoader::Load(const std::string& filename, FBXLoadedModel& out_model)
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

	out_model.meshes.resize(scene->GetGeometryCount());
	mesh_counter_ = 0;

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

bool ba::FBXLoader::LoadNode(FbxNode* node, FBXLoadedModel& out_model)
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

				// Load geometry data.
				if (!LoadGeometry(fbx_mesh, out_model.meshes[mesh_counter_]))
					return false;

				// Get transform.
				LoadTransform(node, out_model.meshes[mesh_counter_].transform);

				// Get materials
				LoadMaterials(node, out_model.meshes[mesh_counter_].materials);

				++mesh_counter_;

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

bool ba::FBXLoader::LoadGeometry(FbxMesh* fbx_mesh, FBXLoadedMesh& out_mesh)
{
	if (!fbx_mesh->IsTriangleMesh())
		return false;

	FbxGeometryElementNormal* normal_element = fbx_mesh->GetElementNormal();
	FbxGeometryElementUV* uv_element = fbx_mesh->GetElementUV();
	FbxGeometryElementMaterial* material_element = fbx_mesh->GetElementMaterial();
	FbxGeometryElementTangent* tangent_element = fbx_mesh->GetElementTangent();
	
	if (!normal_element)
		return false;
	if (!uv_element)
		return false;
	if (!material_element)
		return false;

	out_mesh.b_has_tangent = false;
	if (tangent_element)
		out_mesh.b_has_tangent = true;
	
	// Load informations of vertices.
	//
	FbxVector4* control_points = fbx_mesh->GetControlPoints();

	int tri_count = fbx_mesh->GetPolygonCount();
	out_mesh.vertices.resize(3 * tri_count);
	out_mesh.face_material_indices.resize(tri_count);

	for (int tri_idx = 0; tri_idx < tri_count; ++tri_idx)
	{
		if (!ReadMaterialIndex(material_element, tri_idx, out_mesh.face_material_indices[tri_idx])) return false;

		for (int i = 0; i < 3; ++i)
		{
			int control_point_idx = fbx_mesh->GetPolygonVertex(tri_idx, i);
			int uv_idx = fbx_mesh->GetTextureUVIndex(tri_idx, i);
			int vertex_idx = 3 * tri_idx + i;

			ReadPosition(control_points[control_point_idx], out_mesh.vertices[vertex_idx].pos);
			if (!ReadNormal(normal_element, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].normal)) return false;
			if (!ReadUV(uv_element, control_point_idx, uv_idx, out_mesh.vertices[vertex_idx].uv)) return false;

			if (out_mesh.b_has_tangent)
			{
				if (!ReadTangent(tangent_element, control_point_idx, vertex_idx, out_mesh.vertices[vertex_idx].tangent))
					return false;
			}
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

void ba::FBXLoader::LoadMaterials(FbxNode* node, std::vector<FBXLoadedMaterial>& out_materials)
{
	int node_material_count = node->GetMaterialCount();
	out_materials.resize(node_material_count);

	// 'material_idx' is the index of material "in the node".
	for (int material_idx = 0; material_idx < node_material_count; ++material_idx)
	{
		FbxSurfaceMaterial* fbx_material = node->GetMaterial(material_idx);

		FbxProperty prop;
		FbxDouble3 color;
		FbxDouble factor;

		// Get ambient.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbient);
		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[material_idx].ambient.x = static_cast<float>(color.mData[0]);
		out_materials[material_idx].ambient.y = static_cast<float>(color.mData[1]);
		out_materials[material_idx].ambient.z = static_cast<float>(color.mData[2]);
		//__

		// Get diffuse.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuse);

		LoadTextureInfos(prop, FbxTexture::ETextureUse::eStandard, out_materials[material_idx].texture_infos);

		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[material_idx].diffuse.x = static_cast<float>(color.mData[0]);
		out_materials[material_idx].diffuse.y = static_cast<float>(color.mData[1]);
		out_materials[material_idx].diffuse.z = static_cast<float>(color.mData[2]);
		//__

		// Get specular.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecular);
		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[material_idx].specular.x = static_cast<float>(color.mData[0]);
		out_materials[material_idx].specular.y = static_cast<float>(color.mData[1]);
		out_materials[material_idx].specular.z = static_cast<float>(color.mData[2]);
		//__

		// Get reflection.
		//
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sReflection);
		color = prop.Get<FbxDouble3>();
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sReflectionFactor);
		factor = prop.Get<FbxDouble>();

		out_materials[material_idx].reflection.x = static_cast<float>(color.mData[0]);
		out_materials[material_idx].reflection.y = static_cast<float>(color.mData[1]);
		out_materials[material_idx].reflection.z = static_cast<float>(color.mData[2]);
		//__

		// Get shininess.
		prop = fbx_material->FindProperty(FbxSurfaceMaterial::sShininess);
		FbxDouble shininess = prop.Get<FbxDouble>();

		out_materials[material_idx].shininess = static_cast<float>(shininess);
		//__
	}
}

void ba::FBXLoader::LoadTextureInfos(FbxProperty& prop, FbxTexture::ETextureUse tex_use, std::vector<FBXLoadedTextureInfo>& out_infos)
{
	int layered_tex_count = prop.GetSrcObjectCount<FbxLayeredTexture>();

	FBXLoadedTextureInfo tex_info;
	
	if (layered_tex_count > 0)
	{
		for (int layer_idx = 0; layer_idx < layered_tex_count; ++layer_idx)
		{
			FbxLayeredTexture* layered_tex = prop.GetSrcObject<FbxLayeredTexture>(layer_idx);

			int tex_count = layered_tex->GetSrcObjectCount<FbxFileTexture>();
			for (int tex_idx = 0; tex_idx < tex_count; ++tex_idx)
			{
				FbxFileTexture* file_tex = layered_tex->GetSrcObject<FbxFileTexture>(tex_idx);

				if (!LoadTextureInfo(file_tex, tex_use, tex_info))
					continue;

				out_infos.push_back(tex_info);
			}
		}
	}
	else
	{
		int tex_count = prop.GetSrcObjectCount<FbxTexture>();
		for (int tex_idx = 0; tex_idx < tex_count; ++tex_idx)
		{
			FbxTexture* tex = prop.GetSrcObject<FbxTexture>(tex_idx);
			
			if (!LoadTextureInfo(tex, tex_use, tex_info))
				continue;

			out_infos.push_back(tex_info);
		}
	}
}

void ba::FBXLoader::ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos)
{
	out_pos.x = static_cast<float>(control_point[0]);
	out_pos.y = static_cast<float>(control_point[1]);
	out_pos.z = static_cast<float>(control_point[2]);
}

bool ba::FBXLoader::ReadNormal(const FbxGeometryElementNormal* normal_element, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal)
{
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

bool ba::FBXLoader::ReadUV(const FbxGeometryElementUV* uv_element, int control_point_idx, int uv_idx, XMFLOAT2& out_uv)
{
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

bool ba::FBXLoader::ReadTangent(const FbxGeometryElementTangent* tangent_element, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent)
{
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

bool ba::FBXLoader::ReadMaterialIndex(const FbxGeometryElementMaterial* material_element, int triangle_idx, int& out_material_idx)
{
	switch (material_element->GetMappingMode())
	{
	case FbxGeometryElement::eAllSame:
	{
		// material index is stored in the IndexArray, not the DirectArray.
		out_material_idx = material_element->GetIndexArray().GetAt(0);
		break;
	}
	case FbxGeometryElement::eByPolygon:
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

bool ba::LoadTextureInfo(const FbxTexture* fbx_tex, FbxTexture::ETextureUse tex_use, FBXLoadedTextureInfo& out_info)
{
	// Set file name.
	out_info.file_name = fbx_tex->GetNameOnly();

	if (tex_use != fbx_tex->GetTextureUse())
		return false;

	// Set blend mode of this texture.
	switch (fbx_tex->GetBlendMode())
	{
	case FbxTexture::EBlendMode::eTranslucent:
	{
		out_info.blend_mode = Renderer::kTranslucent;
		break;
	}
	case FbxTexture::EBlendMode::eAdditive:
	{
		out_info.blend_mode = Renderer::kAdditive;
		break;
	}
	case FbxTexture::EBlendMode::eModulate:
	{
		out_info.blend_mode = Renderer::kModulate;
		break;
	}
	case FbxTexture::EBlendMode::eModulate2:
	{
		out_info.blend_mode = Renderer::kModulate2;
		break;
	}
	case FbxTexture::EBlendMode::eOver:
	{
		out_info.blend_mode = Renderer::kOver;
		break;
	}
	default:
		return false;
	}

	// Calculate transform matrix of this texture.
	XMMATRIX scale = XMMatrixScaling(static_cast<float>(fbx_tex->GetScaleU()), static_cast<float>(fbx_tex->GetScaleV()), 1.0f);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(static_cast<float>(fbx_tex->GetRotationV()), static_cast<float>(fbx_tex->GetRotationW()), static_cast<float>(fbx_tex->GetRotationU()));
	XMMATRIX translation = XMMatrixTranslation(static_cast<float>(fbx_tex->GetTranslationU()), static_cast<float>(fbx_tex->GetTranslationV()), 0.0f);

	XMStoreFloat4x4(&out_info.transform, scale * rotation * translation);

	return true;
}
