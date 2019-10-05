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
			XMFLOAT4 ambient;
			XMFLOAT4 diffuse;
			XMFLOAT4 specular;
			XMFLOAT4 reflection;
		};

		struct FBXLoaderMesh
		{
			std::vector<FBXLoaderVertex> vertices;
			XMFLOAT4X4 transform;
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

		bool LoadMesh(FbxMesh* fbx_mesh, FBXLoaderMesh& out_mesh);
		void LoadTransform(FbxNode* node, XMFLOAT4X4& out_transform);

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
