#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class FBXLoader : public Uncopiable
	{
	private:
		FBXLoader();

	public:
		static FBXLoader& GetInstance();

		void Init();
		void Release();

		bool Load(const std::string& filename, Mesh& out_mesh);

	private:
		void Load(FbxNode* node, Mesh& out_mesh);

		void LoadMesh(const FbxMesh* fbx_mesh, Mesh& out_mesh);

		void ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos);
		void ReadNormal(const FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal);
		void ReadTangent(const FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent);

	private:
		FbxManager* manager_;
		FbxIOSettings* io_settings_;
	};
}
