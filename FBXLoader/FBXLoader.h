#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class FBXLoader : public Uncopiable
	{
	public:
		struct FBXLoaderVertex
		{
			XMFLOAT3 pos;
			XMFLOAT3 normal;
			XMFLOAT3 tangent;
			XMFLOAT2 uv;
		};

		struct FBXLoaderMaterial
		{
			XMFLOAT3	ambient;			// ambient color
			float		ambient_factor;		// attenuation of the ambient color
			XMFLOAT3	diffuse;			// diffuse color
			float		diffuse_factor;		// attenuation of the diffuse color
			XMFLOAT3	specular;			// specular color
			float		specular_factor;	// attenuation of the specular color
			XMFLOAT3	reflection;			// reflection color
			float		reflection_factor;	// attenuation of the reflection color
			float		shininess;			// specular exponent in the Phong illumination model
		};
		
		struct FBXLoaderMesh
		{
			std::vector<FBXLoaderVertex>	vertices;
			XMFLOAT4X4						transform;
			FBXLoaderMaterial				material;
		};

		struct FBXLoaderModel
		{
			std::vector<FBXLoaderMesh> meshes;
		};

	private:
		FBXLoader();

	public:
		static FBXLoader& GetInstance();

		void Init();
		void Release();

		bool Load(const std::string& filename, FBXLoaderModel& out_model);

	private:
		bool LoadNode(FbxNode* node, FBXLoaderModel& out_model);

		bool LoadGeometry(FbxMesh* fbx_mesh, FBXLoaderMesh& out_mesh);
		void LoadTransform(FbxNode* node, XMFLOAT4X4& out_transform);
		void LoadMaterial(FbxNode* node, FBXLoaderMaterial& out_material, int material_idx);

		void ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos);
		bool ReadNormal(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal);
		bool ReadTangent(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent);
		bool ReadUV(FbxMesh* fbx_mesh, int control_point_idx, int uv_idx, XMFLOAT2& out_uv);

	private:
		static const int kMaterialCountMax;

		FbxManager*		manager_;
		FbxIOSettings*	io_settings_;
	};
}
