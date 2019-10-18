#pragma once

/*
PCH: Yes
*/

namespace ba
{
	struct FBXLoadedVertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		XMFLOAT3 tangent;
	};

	struct FBXLoadedTextureInfo
	{
		std::string						file_name;
		XMFLOAT4X4						transform;
		renderstates::blend::BlendMode	blend_mode;
	};

	struct FBXLoadedMaterial
	{
		XMFLOAT3	ambient;				// ambient color
		XMFLOAT3	diffuse;				// diffuse color
		XMFLOAT3	specular;				// specular color
		XMFLOAT3	reflection;				// reflection color
		float		shininess;				// specular exponent in the Phong illumination model
		std::vector<FBXLoadedTextureInfo> texture_infos;
	};

	struct FBXLoadedMesh
	{
		std::vector<FBXLoadedVertex>	vertices;
		std::vector<int>				face_material_indices;
		std::vector<FBXLoadedMaterial>	materials;
		XMFLOAT4X4						transform;
		bool							b_has_tangent;
	};

	struct FBXLoadedModel
	{
		std::vector<FBXLoadedMesh>		meshes;
	};
}