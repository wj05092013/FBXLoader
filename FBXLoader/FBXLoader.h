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
			XMFLOAT3	diffuse;			// diffuse color
			XMFLOAT3	specular;			// specular color
			XMFLOAT3	reflection;			// reflection color
			float		shininess;			// specular exponent in the Phong illumination model
		};
		
		struct FBXLoaderMesh
		{
			std::vector<FBXLoaderVertex>	vertices;
			std::vector<FBXLoaderMaterial>	materials;
			std::vector<int>				face_material_indices;
			XMFLOAT4X4						transform;
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
		void LoadMaterials(FbxNode* node, std::vector<FBXLoaderMaterial>& out_materials);

		void ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos);
		bool ReadNormal(FbxLayer* layer, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal);
		bool ReadTangent(FbxLayer* layer, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent);
		bool ReadUV(FbxLayer* layer, int control_point_idx, int uv_idx, XMFLOAT2& out_uv);
		bool ReadMaterialIndex(FbxLayer* layer, int triangle_idx, int& out_material_idx);

	private:
		static const int kMaterialCountMax;

		FbxManager*		manager_;
		FbxIOSettings*	io_settings_;
	};
}
