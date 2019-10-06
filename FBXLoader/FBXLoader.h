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
			XMFLOAT3 ambient;
			float ambient_factor;
			XMFLOAT3 diffuse;
			float diffuse_factor;
			XMFLOAT3 specular;
			float specular_factor;
			XMFLOAT3 reflection;
			float reflection_factor;
		};

		struct FBXLoaderMesh
		{
			std::vector<FBXLoaderVertex> vertices;
			XMFLOAT4X4 transform;
			FBXLoaderMaterial material;
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
		void LoadMaterial(FbxNode* node, FBXLoaderMaterial& out_material);

		void ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos);
		bool ReadNormal(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal);
		bool ReadTangent(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent);
		bool ReadUV(FbxMesh* fbx_mesh, int control_point_idx, int uv_idx, XMFLOAT2& out_uv);
		
		void FbxAMatrixToXMFLOAT4x4(const FbxAMatrix& fbx_m, XMFLOAT4X4& out_m);

	private:
		FbxManager* manager_;
		FbxIOSettings* io_settings_;
	};
}
